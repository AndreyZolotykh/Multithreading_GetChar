#include <iostream> 
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;

char input;
mutex _cout_lock;

void test_get_char(void) {

	cout << "Press q ..." << endl;

	while (true) {
		// Wait for single character 
		input = getchar();

		// Echo input:
		if (input!='\n') 
      cout << "Thread # " << std::this_thread::get_id() << "--" << input << "--" << endl;

		if (input == 'q') {
			cout << " from test_get_char() q char detected" << endl;
			break;
		}
	}
}

void some_work(void) {
	for (auto i = 0; i < 10; i++) {
			{
				std::lock_guard<std::mutex> l(_cout_lock);
				std::cout << "Thread # " << std::this_thread::get_id() << "  some work " << i << std::endl;
				if (input == 'q')
					cout << " from some_work() q char detected" << endl;
			}
			std::chrono::milliseconds duration(1000);
			std::this_thread::sleep_for(duration);
	}
}


int main() {

	thread th1, th2;
	
	th1 = std::thread(test_get_char);
	th2 = std::thread(some_work);

	th1.join();
	th2.join();

	if (input == 'q')
		cout << " from main q char detected" << endl;

	// And we're out of here 
	return 0;
}
