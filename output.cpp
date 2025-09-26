#include "lib/colorama/colorama.hpp"
#include "lib/time/time.hpp"
#include "lib/routines/routines.hpp"

#include "lib/sttlib/sttlib.hpp"
using namespace std;
AtomicInt klaar = 0;
Task benchmarkRoutine(std::string name) {klaar += 1;
co_return;}void thread_starter() {
}
int main() {thread_starter();
thread_starter();
int numR = 500000;
int time = 0;
Scheduler scheduler;
scheduler.run();
time_utils::Stopwatch sw;
sw.start();
for (int teller  : range(0,numR,1)) {
    scheduler.spawn(benchmarkRoutine, ("bench" + to_string(teller)));
}
while ((klaar < numR)) {
    sleep(1);
}
color("white", "bold");
print(sw.elapsed(), "sec");
scheduler.stop_scheduler();
scheduler.join();
}
