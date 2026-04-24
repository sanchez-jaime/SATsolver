#!/bin/bash
#
# run_benchmarks.sh — Run ./mySAT against every .cnf file in ./simple_benchmarks
#
# Usage:
#   ./run_benchmarks.sh                  # run all benchmarks
#   ./run_benchmarks.sh -t 60            # set a 60-second timeout per benchmark
#   ./run_benchmarks.sh -d path/to/dir   # use a different benchmark directory
#
 
set -u
 
# -------- Defaults --------
SOLVER="./mySAT"
BENCH_DIR="./simple_benchmarks"
TIMEOUT_SECS=0          # 0 = no timeout
LOG_ROOT="./benchmark_logs"
 
# -------- Parse args --------
while getopts "d:t:s:l:h" opt; do
    case "$opt" in
        d) BENCH_DIR="$OPTARG" ;;
        t) TIMEOUT_SECS="$OPTARG" ;;
        s) SOLVER="$OPTARG" ;;
        l) LOG_ROOT="$OPTARG" ;;
        h)
            echo "Usage: $0 [-d benchmark_dir] [-t timeout_secs] [-s solver] [-l log_root]"
            exit 0
            ;;
        *) echo "Invalid option. Use -h for help." >&2; exit 1 ;;
    esac
done
 
# -------- Sanity checks --------
if [[ ! -x "$SOLVER" ]]; then
    echo "Error: solver '$SOLVER' not found or not executable." >&2
    exit 1
fi
 
if [[ ! -d "$BENCH_DIR" ]]; then
    echo "Error: benchmark directory '$BENCH_DIR' does not exist." >&2
    exit 1
fi
 
# Create a new timestamped subdirectory for this run
TIMESTAMP=$(date +%Y-%m-%d_%H-%M-%S)
LOG_DIR="$LOG_ROOT/$TIMESTAMP"
mkdir -p "$LOG_DIR"
 
# Collect .cnf files
shopt -s nullglob
benchmarks=("$BENCH_DIR"/*.cnf)
shopt -u nullglob
 
if (( ${#benchmarks[@]} == 0 )); then
    echo "No .cnf files found in $BENCH_DIR"
    exit 0
fi
 
# -------- Run --------
total=${#benchmarks[@]}
passed=0
failed=0
timed_out=0
index=0
 
SUMMARY_FILE="$LOG_DIR/summary.txt"
 
printf "Running %d benchmark(s) with %s\n" "$total" "$SOLVER"
printf "Logs will be written to %s\n\n" "$LOG_DIR"
 
{
    echo "Run started: $(date)"
    echo "Solver:      $SOLVER"
    echo "Benchmarks:  $BENCH_DIR"
    echo "Timeout:     ${TIMEOUT_SECS}s"
    echo
    printf "%-40s %-10s %-10s\n" "BENCHMARK" "RESULT" "TIME(s)"
    printf "%-40s %-10s %-10s\n" "----------------------------------------" "----------" "----------"
} > "$SUMMARY_FILE"
 
for bench in "${benchmarks[@]}"; do
    index=$((index + 1))
    name=$(basename "$bench")
    log_file="$LOG_DIR/${name%.cnf}.log"
 
    printf "[%d/%d] %s ... " "$index" "$total" "$name"
 
    start=$(date +%s.%N)
    if (( TIMEOUT_SECS > 0 )); then
        timeout "${TIMEOUT_SECS}s" "$SOLVER" "$bench" > "$log_file" 2>&1
        status=$?
    else
        "$SOLVER" "$bench" > "$log_file" 2>&1
        status=$?
    fi
    end=$(date +%s.%N)
 
    elapsed=$(awk -v s="$start" -v e="$end" 'BEGIN { printf "%.2f", e - s }')
 
    if (( status == 124 )); then
        result="TIMEOUT"
        printf "TIMEOUT (%ss)\n" "$elapsed"
        timed_out=$((timed_out + 1))
    elif (( status == 0 )); then
        result="OK"
        printf "OK (%ss)\n" "$elapsed"
        passed=$((passed + 1))
    else
        result="FAIL($status)"
        printf "FAILED exit=%d (%ss)\n" "$status" "$elapsed"
        failed=$((failed + 1))
    fi
 
    printf "%-40s %-10s %-10s\n" "$name" "$result" "$elapsed" >> "$SUMMARY_FILE"
done
 
# -------- Summary --------
{
    echo
    echo "================ Summary ================"
    printf "Total:    %d\n" "$total"
    printf "Passed:   %d\n" "$passed"
    printf "Failed:   %d\n" "$failed"
    printf "Timeouts: %d\n" "$timed_out"
    echo "Run finished: $(date)"
    echo "========================================="
} | tee -a "$SUMMARY_FILE"
 
echo
echo "Results saved to: $LOG_DIR"
 
# Non-zero exit if any benchmark failed/timed out
(( failed == 0 && timed_out == 0 ))