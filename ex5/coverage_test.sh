#!/bin/bash

# Coverage test script for Part 4 - Using Makefile
TCP_PORT=12345
UDP_PORT=12346

# Clean old coverage files first to avoid stamp mismatch
echo "Cleaning old coverage files..."
rm -f *.gcda *.gcno *.gcov

echo "Building with coverage using Makefile..."
if ! make clean > /dev/null 2>&1; then
    echo "ERROR: make clean failed"
    exit 1
fi

if ! make all > /dev/null 2>&1; then
    echo "ERROR: make all failed"
    exit 1
fi

echo "Build successful!"

echo "Checking built files..."
ls -la atom_supplier drinks_bar molecule_requestor *.gcno 2>/dev/null

echo "Running tests..."

# Start molecule_supplier server (using drinks_bar)
echo "Starting molecule supplier server on ports $TCP_PORT (TCP) and $UDP_PORT (UDP)..."
./drinks_bar -T $TCP_PORT -U $UDP_PORT -c 100 -h 200 -o 150 &
SERVER_PID=$!
sleep 2

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start"
    exit 1
fi

echo "Server started (PID: $SERVER_PID)"

# ATOM_SUPPLIER TESTS

echo "Test 1: No arguments"
./atom_supplier 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test 2: Missing port"
./atom_supplier -h 127.0.0.1 2>/dev/null || echo "✓ Correctly rejected missing port"

echo "Test 3: Invalid IP"
./atom_supplier -h 999.999.999.999 -p $TCP_PORT 2>/dev/null || echo "✓ Correctly rejected invalid IP"

echo "Test 4: Quick connection test"
timeout 3 bash -c "echo 'EXIT' | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT" 2>/dev/null || echo "✓ Connection test done"

echo "Test 5: Send message to server"
timeout 3 bash -c "echo 'ADD CARBON 100' | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT" 2>/dev/null || echo "✓ Message test done"

echo "Test 5b: Send HYDROGEN to server"
timeout 3 bash -c "echo 'ADD HYDROGEN 100' | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT" 2>/dev/null || echo "✓ HYDROGEN test done"

echo "Test 5c: Send OXYGEN to server"
timeout 3 bash -c "echo 'ADD OXYGEN 50' | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT" 2>/dev/null || echo "✓ OXYGEN test done"

echo "Test 6: Connect to non-existent server"
./atom_supplier -h 127.0.0.1 -p 9999 2>/dev/null || echo "✓ Correctly failed to connect"

echo "Test 7: send non Exist message"
timeout 3 bash -c "
(echo 'HELLO'; sleep 0.2; echo 'EXIT') | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT
" 2>/dev/null || echo "✓ Correctly handled non-existent message"

echo "Test 7b: Unix socket test"
./atom_supplier -f /nonexistent/path 2>/dev/null || echo "✓ Unix socket test done"

echo "Test 7c: Conflicting arguments"
./atom_supplier -h 127.0.0.1 -p $TCP_PORT -f /tmp/socket 2>/dev/null || echo "✓ Correctly rejected conflicting args"

echo "Test 7d: Invalid option"
./atom_supplier -x 2>/dev/null || echo "✓ Correctly rejected invalid option"

echo "Test 8: server shutdown"
timeout 10 bash -c "
# Start atom_supplier in background
(echo 'HELLO'; sleep 5; echo 'EXIT') | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT &
SUPPLIER_PID=\$!
# Wait a bit then kill the server to trigger server disconnect
sleep 2
kill -SIGINT $SERVER_PID
wait \$SUPPLIER_PID
" 2>/dev/null || echo "✓ Correctly handled server shutdown"

# Restart server for remaining tests
echo "Restarting server..."
./drinks_bar -T $TCP_PORT -U $UDP_PORT -c 100 -h 200 -o 150 &
SERVER_PID=$!
sleep 2

echo "Test 9: EOF simulation"  
timeout 2 bash -c "
exec 0</dev/null
./atom_supplier -h 127.0.0.1 -p $TCP_PORT 2>/dev/null
" || echo "✓ EOF detection test done"

echo "Test 10: Unix socket success"
timeout 5 bash -c "
# Start server with Unix sockets
./drinks_bar -s /tmp/uds_test -d /tmp/uds_dgram -c 5 -h 5 -o 5 &
UDS_PID=\$!
sleep 1

# Test successful Unix connection  
echo 'EXIT' | ./atom_supplier -f /tmp/uds_test 2>/dev/null || true

# Cleanup
kill -SIGINT \$UDS_PID 2>/dev/null
wait \$UDS_PID 2>/dev/null
rm -f /tmp/uds_test /tmp/uds_dgram
" || echo "✓ Unix socket success test done"

echo "=== MOLECULE_REQUESTOR TESTS ==="

echo "Test R1: No arguments"
./molecule_requestor 2>/dev/null || echo "✓ Correctly rejected no args"

echo "Test R2: Invalid port"
./molecule_requestor -h 127.0.0.1 -p 0 2>/dev/null || echo "✓ Correctly rejected port 0"

echo "Test R3: Invalid IP"
./molecule_requestor -h 999.999.999.999 -p $UDP_PORT 2>/dev/null || echo "✓ Correctly rejected invalid IP"

echo "Test R4: DELIVER WATER"
timeout 5 bash -c "(echo 'DELIVER WATER 2'; sleep 1; echo 'quit') | ./molecule_requestor -h 127.0.0.1 -p $UDP_PORT" 2>/dev/null || echo "✓ WATER delivery test done"

echo "Test R5: DELIVER CARBON DIOXIDE"
timeout 5 bash -c "(echo 'DELIVER CARBON DIOXIDE 1'; sleep 1; echo 'quit') | ./molecule_requestor -h 127.0.0.1 -p $UDP_PORT" 2>/dev/null || echo "✓ CO2 delivery test done"

echo "Test R6: Unix socket test"
echo "quit" | timeout 3 ./molecule_requestor -f /nonexistent/path 2>/dev/null || echo "✓ Unix socket test done"

echo "Test R7: Conflicting arguments"
./molecule_requestor -h 127.0.0.1 -p $UDP_PORT -f /tmp/socket 2>/dev/null || echo "✓ Correctly rejected conflicting args"

echo "Test R8: Invalid option"
./molecule_requestor -x 2>/dev/null || echo "✓ Correctly rejected invalid option"

echo "Test R9: EOF detection for molecule_requestor"
printf "" | timeout 2 ./molecule_requestor -h 127.0.0.1 -p 9999 2>/dev/null || echo "✓ EOF test done"

# MOLECULE_SUPPLIER TESTS (using drinks_bar)
echo "=== MOLECULE_SUPPLIER TESTS ==="
echo "Test M1: No arguments"
./drinks_bar 2>/dev/null || echo "✓ Correctly rejected no args"
echo "Test M2: Invalid port"
./drinks_bar -T 0 -U 8081 2>/dev/null || echo "✓ Correctly rejected port 0"
echo "Test M3: Bind error - port already in use"
timeout 3 bash -c "
./drinks_bar -T $TCP_PORT -U $UDP_PORT 2>/dev/null &
sleep 0.5
./drinks_bar -T $TCP_PORT -U $UDP_PORT 2>/dev/null # This should fail with bind error
" || echo "✓ Bind error test done"

echo "Test M9: DELIVER CARBON DIOXIDE with sufficient atoms"
timeout 10 bash -c '
./drinks_bar -T 8094 -U 8095 -c 10 -h 10 -o 10 & # Use new unique ports with initial atoms
SERVER_PID=$!
sleep 1

# Add sufficient atoms for CARBON DIOXIDE (1C + 2O)
echo "ADD CARBON 1" | ./atom_supplier -h 127.0.0.1 -p 8094 &
sleep 0.5
echo "ADD OXYGEN 2" | ./atom_supplier -h 127.0.0.1 -p 8094 &
sleep 2

# Request CARBON DIOXIDE delivery via UDP
(echo "DELIVER CARBON DIOXIDE 1"; sleep 1; echo "quit") | ./molecule_requestor -h 127.0.0.1 -p 8095 &
sleep 2

# Clean shutdown
kill -SIGINT $SERVER_PID
wait $SERVER_PID 2>/dev/null
' || echo "✓ CARBON DIOXIDE delivery with sufficient atoms test completed"

echo "Test MR2: Invalid IP address"
./molecule_requestor -h "999.999.999.999" -p 9999 2>/dev/null || echo "✓ Invalid IP test done"

echo "Test: Send to existing server but no response"
timeout 18 bash -c '
(
  echo "DELIVER WATER 1"  # Send to real server
  sleep 15               # Wait for timeout
  echo "quit"
) | ./molecule_requestor -h 127.0.0.1 -p '"$UDP_PORT"' 2>/dev/null
' || echo "✓ Timeout with real server"

# Test M10: Max clients reached
echo "Test M10: Max clients reached"
timeout 10 bash -c '
./drinks_bar -T 8070 -U 8071 -c 50 -h 50 -o 50 &
SERVER_PID=$!
sleep 1

# Open 12 clients (MAX_CLIENTS) that stay connected
for i in $(seq 1 12); do
    (echo "ADD CARBON 1"; sleep 8) | ./atom_supplier -h 127.0.0.1 -p 8070 &
    sleep 0.1
done

sleep 1

# Try 13th client - should be rejected
echo "ADD HYDROGEN 1" | ./atom_supplier -h 127.0.0.1 -p 8070 &
sleep 2

kill -SIGINT $SERVER_PID
wait $SERVER_PID 2>/dev/null
' || echo "✓ Max clients reached test completed"

echo "Test: Initial atoms"
timeout 3 bash -c './drinks_bar -T 8080 -U 8081 -c 5 -h 10 -o 3 &
SERVER_PID=$!
sleep 1
kill -SIGINT $SERVER_PID
wait $SERVER_PID 2>/dev/null' || echo "✓ Initial atoms test done"

echo "Test: Successful delivery"
timeout 5 bash -c '
./drinks_bar -T 8080 -U 8081 -c 10 -h 10 -o 10 &
SERVER_PID=$!
sleep 1
(echo "DELIVER WATER 1"; sleep 1; echo "quit") | ./molecule_requestor -h 127.0.0.1 -p 8081 &
sleep 3
kill -SIGINT $SERVER_PID
wait $SERVER_PID 2>/dev/null' || echo "✓ Successful delivery test done"

echo "Test: Unknown atom type" 
timeout 3 bash -c "echo 'ADD FOOBAR 5' | ./atom_supplier -h 127.0.0.1 -p $TCP_PORT" 2>/dev/null || echo "✓ Unknown atom test done"

echo "Test: Negative carbon value"
./drinks_bar -T 8080 -U 8081 -c -5 2>/dev/null || echo "✓ Negative carbon test done"

echo "Test: Negative oxygen"
./drinks_bar -T 8080 -U 8081 -o -5 2>/dev/null || echo "✓ Negative oxygen test done"

echo "Test: Negative hydrogen"
./drinks_bar -T 8080 -U 8081 -h -5 2>/dev/null || echo "✓ Negative hydrogen test done"

echo "Test: Unknown molecule"
timeout 3 bash -c "(echo 'DELIVER BLABLA 1'; sleep 1; echo 'quit') | ./molecule_requestor -h 127.0.0.1 -p $UDP_PORT" 2>/dev/null || echo "✓ Unknown molecule test done"

echo "Test: Timeout option"
timeout 3 bash -c './drinks_bar -T 8080 -U 8081 -t 2 &
SERVER_PID=$!
sleep 1
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null' || echo "✓ Timeout test done"

echo "Test: Generate VODKA"
timeout 8 bash -c '
{
    sleep 1
    echo "GEN VODKA"
    sleep 2
} | ./drinks_bar -T 8080 -U 8081 -c 10 -h 20 -o 8 &
SERVER_PID=$!
sleep 5
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ VODKA generation test done"

echo "Test: Generate CHAMPAGNE"
timeout 8 bash -c '
{
    sleep 1
    echo "GEN CHAMPAGNE"
    sleep 2
} | ./drinks_bar -T 8080 -U 8081 -c 5 -h 10 -o 5 &
SERVER_PID=$!
sleep 5
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ CHAMPAGNE generation test done"

echo "Test: Generate SOFT DRINK"
timeout 8 bash -c '
{
   sleep 1
   echo "GEN SOFT DRINK"
   sleep 2
} | ./drinks_bar -T 8080 -U 8081 -c 7 -h 14 -o 9 &
SERVER_PID=$!
sleep 5
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ SOFT DRINK generation test done"

echo "Test: UDP message with newline"
timeout 5 bash -c '
./drinks_bar -T 9996 -U 9995 -c 5 -h 5 -o 5 &
SERVER_PID=$!
sleep 1
printf "DELIVER WATER 1\n" | nc -u -w1 127.0.0.1 9995 2>/dev/null || true
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ UDP newline test done"

echo "Test: Not enough atoms for delivery"
timeout 5 bash -c '
./drinks_bar -T 8080 -U 8081 -c 1 -h 1 -o 1 &
SERVER_PID=$!
sleep 1
(echo "DELIVER GLUCOSE 1"; sleep 1; echo "quit") | ./molecule_requestor -h 127.0.0.1 -p 8081 2>/dev/null || true
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ Not enough atoms test done"

echo "Test: Not enough atoms for drink generation"
timeout 5 bash -c '
{
    sleep 1
    echo "GEN VODKA"
    sleep 2
} | ./drinks_bar -T 8080 -U 8081 -c 1 -h 1 -o 1 &
SERVER_PID=$!
sleep 4
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ Not enough atoms for drink test done"

echo "Test: Invalid stdin command"
timeout 5 bash -c '
{
    sleep 1
    echo "INVALID COMMAND"
    sleep 2
} | ./drinks_bar -T 8080 -U 8081 -c 5 -h 5 -o 5 &
SERVER_PID=$!
sleep 4
kill -SIGINT $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
' || echo "✓ Invalid command test done"

echo "Test: Conflicting socket types"
./drinks_bar -T 8080 -U 8081 -s /tmp/stream -d /tmp/dgram 2>/dev/null || echo "✓ Conflicting socket types test done"



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
pkill -f "drinks_bar" 2>/dev/null || true
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
calculate_coverage "drinks_bar.c"