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

# Test cases with timeout
echo "Running test 1..."
timeout 5 bash -c "echo -e 'ADD CARBON 10\nADD HYDROGEN 5\nADD OXYGEN 3\nEXIT' | ./supplier 127.0.0.1 $PORT" || echo "Test 1 timeout/error"
sleep 1

echo "Running test 2..."
timeout 5 bash -c "echo -e 'ADD URANIUM 5\nEXIT' | ./supplier 127.0.0.1 $PORT" || echo "Test 2 timeout/error"
sleep 1

echo "Running test 3..."
timeout 5 bash -c "echo -e 'INVALID\nEXIT' | ./supplier 127.0.0.1 $PORT" || echo "Test 3 timeout/error"
sleep 1

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
calculate_coverage "atom_warehouse.c"