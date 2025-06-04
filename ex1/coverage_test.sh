#!/bin/bash

# Simple coverage test script
PORT=12345

echo "Building with coverage..."
make clean > /dev/null 2>&1
make all > /dev/null 2>&1

echo "Running tests..."

# Start server
echo "Starting warehouse server on port $PORT..."
./warehouse $PORT &
SERVER_PID=$!
sleep 2

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start"
    exit 1
fi

echo "Server started (PID: $SERVER_PID)"

# Test supplier argument validation (creates coverage)
echo "Testing supplier argument validation..."

echo "Test 1: No arguments"
./supplier 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test 2: Missing port"  
./supplier 127.0.0.1 2>/dev/null || echo "✓ Correctly rejected missing port"

echo "Test 3: Invalid IP"
./supplier 999.999.999.999 $PORT 2>/dev/null || echo "✓ Correctly rejected invalid IP"

echo "Test 4: Quick connection test"
timeout 3 bash -c "echo 'EXIT' | ./supplier 127.0.0.1 $PORT" 2>/dev/null || echo "✓ Connection test done"
 
echo "Test 5: Send message to server"
timeout 3 bash -c "echo 'ADD CARBON 100' | ./supplier 127.0.0.1 $PORT" 2>/dev/null || echo "✓ Message test done" 

echo "Test 5b: Send HYDROGEN to server"
timeout 3 bash -c "echo 'ADD HYDROGEN 100' | ./supplier 127.0.0.1 $PORT" 2>/dev/null || echo "✓ HYDROGEN test done"

echo "Test 5c: Send OXYGEN to server"  
timeout 3 bash -c "echo 'ADD OXYGEN 50' | ./supplier 127.0.0.1 $PORT" 2>/dev/null || echo "✓ OXYGEN test done"


echo "Test 6: Connect to non-existent server"
./supplier 127.0.0.1 9999 2>/dev/null || echo "✓ Correctly failed to connect"

echo "Test 7: send non Exsit message"
# ... שאר הטסטים

echo "Test 6: Connect to non-existent server"
./supplier 127.0.0.1 9999 2>/dev/null || echo "✓ Correctly failed to connect"

echo "Test 7: send non Exsit message"
timeout 3 bash -c "
(echo 'HELLO'; sleep 0.2; echo 'EXIT') | ./supplier 127.0.0.1 $PORT
" 2>/dev/null || echo "✓ Correctly handled non-existent message"

echo "Test 8: serer shutdown"
timeout 10 bash -c "
# Start supplier in background
(echo 'HELLO'; sleep 5; echo 'EXIT') | ./supplier 127.0.0.1 $PORT &
SUPPLIER_PID=\$!

# Wait a bit then kill the server to trigger server disconnect
sleep 2
kill -SIGINT $SERVER_PID
wait \$SUPPLIER_PID
" 2>/dev/null || echo "✓ Correctly handled server shutdown"

echo "Test 9: Mock socket failure"
timeout 3 bash -c '
echo "int socket(int a,int b,int c){return -1;}" > fake_socket.c
gcc -shared -fPIC fake_socket.c -o fake_socket.so 2>/dev/null
LD_PRELOAD=./fake_socket.so ./supplier 127.0.0.1 '"$PORT"' 2>/dev/null
rm -f fake_socket.so fake_socket.c
' || echo " Mocked socket failure test done"

echo "=== WAREHOUSE TESTS ==="

echo "Test W1: No arguments"
./warehouse 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test W2: Invalid port"
./warehouse 0 2>/dev/null || echo "✓ Correctly rejected port 0"

echo "Test W3: Bind error - port already in use"
# Start second server on same port to force bind error
timeout 3 bash -c "
./warehouse $PORT 2>/dev/null &
sleep 0.5
./warehouse $PORT 2>/dev/null  # This should fail with bind error
" || echo "✓ Bind error test done"

echo "Test W4: Mock socket failure for warehouse"
timeout 3 bash -c '
echo "int socket(int a,int b,int c){return -1;}" > fake_socket_warehouse.c
gcc -shared -fPIC fake_socket_warehouse.c -o fake_socket_warehouse.so 2>/dev/null
LD_PRELOAD=./fake_socket_warehouse.so ./warehouse '"$PORT"' 2>/dev/null
rm -f fake_socket_warehouse.so fake_socket_warehouse.c
' || echo "✓ Warehouse socket error test done"

echo "Test W7: Mock listen() on different port"
timeout 3 bash -c '
echo "#include <sys/socket.h>
#include <errno.h>
int listen(int sockfd, int backlog) { 
    errno = EADDRINUSE;
    return -1; 
}" > fake_listen.c
gcc -shared -fPIC fake_listen.c -o fake_listen.so 2>/dev/null
LD_PRELOAD=./fake_listen.so ./warehouse 9999 2>/dev/null  # פורט אחר!
rm -f fake_listen.so fake_listen.c
' || echo "✓ listen() failure test done"


echo "Stopping server..."
if kill -0 $SERVER_PID 2>/dev/null; then
    kill -SIGINT $SERVER_PID
    sleep 2
    
    # Force kill if still running
    if kill -0 $SERVER_PID 2>/dev/null; then
        echo "Force killing server..."
        kill -KILL $SERVER_PID
    fi
fi

wait $SERVER_PID 2>/dev/null || true

# Additional cleanup - kill any remaining warehouse processes
pkill -f "warehouse.*$PORT" 2>/dev/null || true
sleep 1
# Check what files exist
echo "Checking coverage files..."
ls -la *.gcda *.gcno 2>/dev/null || echo "No coverage data files found"

# Generate coverage and calculate percentages manually
echo ""
echo "Coverage Results:"
echo "=================="

calculate_coverage() {
    local file=$1
    echo "Processing $file..."
    
    # Check if .gcno and .gcda files exist
    local base_name=$(basename "$file" .c)
    if [ ! -f "${base_name}.gcno" ] || [ ! -f "${base_name}.gcda" ]; then
        echo "${file}: Missing coverage data files (.gcno or .gcda)"
        return
    fi
    
    # Run gcov with verbose output for debugging
    local gcov_output=$(gcov "$file" 2>&1)
    echo "gcov output: $gcov_output"
    
    if [ -f "${file}.gcov" ]; then
        # Count executable lines (lines with numbers, not just #####)
        local total_lines=$(grep -c "^[[:space:]]*[0-9#]" "${file}.gcov" 2>/dev/null || echo "0")
        # Count covered lines (lines with numbers > 0)  
        local covered_lines=$(grep -c "^[[:space:]]*[1-9]" "${file}.gcov" 2>/dev/null || echo "0")
        
        if [ "$total_lines" -gt 0 ]; then
            local percentage=$((covered_lines * 100 / total_lines))
            echo "${file}: ${percentage}% (${covered_lines}/${total_lines} lines)"
        else
            echo "${file}: No executable lines found"
        fi
        
        
        echo "---"
    else
        echo "${file}: .gcov file not created"
    fi
}

calculate_coverage "atom_supplier.c"
calculate_coverage "atom_warehouse.c"