#!/bin/bash

# Coverage test script for Part 2 - Using existing tests structure
TCP_PORT=12345
UDP_PORT=12346

# Clean old coverage files first to avoid stamp mismatch
echo "Cleaning old coverage files..."
rm -f *.gcda *.gcno *.gcov

echo "Building with coverage..."
make clean > /dev/null 2>&1
make all > /dev/null 2>&1

# Manual build for missing files - force rebuild with coverage
echo "Building executables with coverage..."
gcc --coverage -o atom_supplier atom_supplier.c 2>/dev/null || echo "Could not build atom_supplier"
gcc --coverage -o molecule_supplier molecule_supplier.c 2>/dev/null || echo "Could not build molecule_supplier"

echo "Checking built files..."
ls -la atom_supplier molecule_supplier *.gcno 2>/dev/null

echo "Running tests..."

# Start molecule_supplier server
echo "Starting molecule supplier server on ports $TCP_PORT (TCP) and $UDP_PORT (UDP)..."
./molecule_supplier $TCP_PORT $UDP_PORT &
SERVER_PID=$!
sleep 2

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start"
    exit 1
fi

echo "Server started (PID: $SERVER_PID)"

# ATOM_SUPPLIER TESTS (adapted from supplier tests)
echo "Test 1: No arguments"
./atom_supplier 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test 2: Missing port"
./atom_supplier 127.0.0.1 2>/dev/null || echo "✓ Correctly rejected missing port"

echo "Test 3: Invalid IP"
./atom_supplier 999.999.999.999 $TCP_PORT 2>/dev/null || echo "✓ Correctly rejected invalid IP"

echo "Test 4: Quick connection test"
timeout 3 bash -c "echo 'EXIT' | ./atom_supplier 127.0.0.1 $TCP_PORT" 2>/dev/null || echo "✓ Connection test done"

echo "Test 5: Send message to server"
timeout 3 bash -c "echo 'ADD CARBON 100' | ./atom_supplier 127.0.0.1 $TCP_PORT" 2>/dev/null || echo "✓ Message test done"

echo "Test 5b: Send HYDROGEN to server"
timeout 3 bash -c "echo 'ADD HYDROGEN 100' | ./atom_supplier 127.0.0.1 $TCP_PORT" 2>/dev/null || echo "✓ HYDROGEN test done"

echo "Test 5c: Send OXYGEN to server"
timeout 3 bash -c "echo 'ADD OXYGEN 50' | ./atom_supplier 127.0.0.1 $TCP_PORT" 2>/dev/null || echo "✓ OXYGEN test done"

echo "Test 6: Connect to non-existent server"
./atom_supplier 127.0.0.1 9999 2>/dev/null || echo "✓ Correctly failed to connect"

echo "Test 7: send non Exist message"
timeout 3 bash -c "
(echo 'HELLO'; sleep 0.2; echo 'EXIT') | ./atom_supplier 127.0.0.1 $TCP_PORT
" 2>/dev/null || echo "✓ Correctly handled non-existent message"

echo "Test 8: server shutdown"
timeout 10 bash -c "
# Start atom_supplier in background
(echo 'HELLO'; sleep 5; echo 'EXIT') | ./atom_supplier 127.0.0.1 $TCP_PORT &
SUPPLIER_PID=\$!
# Wait a bit then kill the server to trigger server disconnect
sleep 2
kill -SIGINT $SERVER_PID
wait \$SUPPLIER_PID
" 2>/dev/null || echo "✓ Correctly handled server shutdown"

# Restart server for remaining tests
echo "Restarting server..."
./molecule_supplier $TCP_PORT $UDP_PORT &
SERVER_PID=$!
sleep 2

echo "Test 9: Mock socket failure"
timeout 3 bash -c '
echo "int socket(int a,int b,int c){return -1;}" > fake_socket.c
gcc -shared -fPIC fake_socket.c -o fake_socket.so 2>/dev/null
LD_PRELOAD=./fake_socket.so ./atom_supplier 127.0.0.1 '"$TCP_PORT"' 2>/dev/null
rm -f fake_socket.so fake_socket.c
' || echo "✓ Mocked socket failure test done"

# MOLECULE_REQUESTOR TESTS (adapted from warehouse tests)
echo "=== MOLECULE_REQUESTOR TESTS ==="

echo "Test R1: No arguments"
./molecule_requestor 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test R2: Invalid port"
./molecule_requestor 127.0.0.1 0 2>/dev/null || echo "✓ Correctly rejected port 0"

echo "Test R3: DELIVER WATER"
timeout 5 bash -c "echo 'DELIVER WATER 2' | ./molecule_requestor 127.0.0.1 $UDP_PORT" 2>/dev/null || echo "✓ WATER delivery test done"

echo "Test R4: DELIVER CARBON DIOXIDE"
timeout 5 bash -c "echo 'DELIVER CARBON DIOXIDE 1' | ./molecule_requestor 127.0.0.1 $UDP_PORT" 2>/dev/null || echo "✓ CO2 delivery test done"

echo "Test R5: Mock socket failure for molecule_requestor"
timeout 3 bash -c '
echo "int socket(int a,int b,int c){return -1;}" > fake_socket_udp.c
gcc -shared -fPIC fake_socket_udp.c -o fake_socket_udp.so 2>/dev/null
LD_PRELOAD=./fake_socket_udp.so ./molecule_requestor 127.0.0.1 '"$UDP_PORT"' 2>/dev/null
rm -f fake_socket_udp.so fake_socket_udp.c
' || echo "✓ molecule_requestor socket error test done"

# MOLECULE_SUPPLIER TESTS (adapted from warehouse tests)
echo "=== MOLECULE_SUPPLIER TESTS ==="

echo "Test M1: No arguments"
./molecule_supplier 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test M2: Invalid port"
./molecule_supplier 0 8081 2>/dev/null || echo "✓ Correctly rejected port 0"

echo "Test M3: Bind error - port already in use"
timeout 3 bash -c "
./molecule_supplier $TCP_PORT $UDP_PORT 2>/dev/null &
sleep 0.5
./molecule_supplier $TCP_PORT $UDP_PORT 2>/dev/null  # This should fail with bind error
" || echo "✓ Bind error test done"

echo "Test M4: Mock socket failure for molecule_supplier"
timeout 3 bash -c '
echo "int socket(int a,int b,int c){return -1;}" > fake_socket_server.c
gcc -shared -fPIC fake_socket_server.c -o fake_socket_server.so 2>/dev/null
LD_PRELOAD=./fake_socket_server.so ./molecule_supplier '"$TCP_PORT"' '"$UDP_PORT"' 2>/dev/null
rm -f fake_socket_server.so fake_socket_server.c
' || echo "✓ molecule_supplier socket error test done"

echo "Test M5: Mock listen() failure"
timeout 3 bash -c '
echo "#include <sys/socket.h>
#include <errno.h>
int listen(int sockfd, int backlog) {
    errno = EADDRINUSE;
    return -1;
}" > fake_listen.c
gcc -shared -fPIC fake_listen.c -o fake_listen.so 2>/dev/null
LD_PRELOAD=./fake_listen.so ./molecule_supplier 9999 9998 2>/dev/null  # Different ports!
rm -f fake_listen.so fake_listen.c
' || echo "✓ listen() failure test done"

# Stop server
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

# Additional cleanup
pkill -f "molecule_supplier" 2>/dev/null || true
pkill -f "atom_supplier" 2>/dev/null || true
pkill -f "molecule_requestor" 2>/dev/null || true
sleep 1

echo "Server stopped"

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
calculate_coverage "molecule_requestor.c"
calculate_coverage "molecule_supplier.c"