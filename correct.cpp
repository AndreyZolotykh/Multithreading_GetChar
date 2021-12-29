#include <iostream>
#include <queue>
#include <thread>
#include <condition_variable>
#include <shared_mutex>
#include <atomic>
#include <mutex>
using namespace std;

struct Node
{
    int _value;
    Node* _next;
    std::mutex node_mutex;
    Node(int value) : _value(value), _next{ nullptr }{}
};

class FineGrainedQueue
{
private:
    Node* _head;
    std::mutex queue_mutex;

public:
    FineGrainedQueue() : _head(nullptr) {}

    void push_back(int data)
    {
        // ñîçäàåì íîâûé óçåë
        Node* node = new Node(data);
        // åñëè ñïèñîê ïóñò, âîçâðàùàåì óçåë
        if (_head == nullptr)
        {
            _head = node;
            return;
        }
        // â öèêëå èùåì ïîñëåäíèé ýëåìåíò ñïèñêà
        Node* last = _head;
        while (last->_next != nullptr)
        {
            last = last->_next;
        }
        // Îáíîâëÿåì óêàçàòåëü next ïîñëåäíåãî óçëà íà óêàçàòåëü íà íîâûé óçåë
        last->_next = node;
        return;
    }

    void remove(int value)
    {
        Node* prev, * cur; // óêàçàòåëè íà ïðåäûäóùèé è òåêóùèé ýëåìåíò
        queue_mutex.lock();

        if (!_head)
            return;
        if (_head && _head->_value == value)
        {
            Node* temp = _head;
            _head = _head->_next;
            delete temp;
            queue_mutex.unlock();
            return;
        }

        prev = this->_head; // ñòàðò: ïðåäûäóùèé = íà÷àëî ñïèñêà
        cur = this->_head->_next; // ñòàðò: òåêóùèé = âòîðîé ýëåìåíò ñïèñêà   
        prev->node_mutex.lock();
        queue_mutex.unlock();

        cur->node_mutex.lock();

        while (cur) // ïîêà óêàçàòåëü íà òåêóùèé ýëåìåíò íå íóëåâîé... 
        {
            if (cur->_value == value) // åñëè íàøëè ýëåìåíò äëÿ óäàëåíèÿ... 
            {
                // òî óäàëÿåì åãî
                prev->_next = cur->_next;
                prev->node_mutex.unlock();
                cur->node_mutex.unlock();
                delete cur;
                return;
            }

            // åñëè íå íàøëè ýëåìåíò äëÿ óäàëåíèÿ, òî äâèãàåìñÿ äàëüøå ïî î÷åðåäè
            Node* old_prev = prev;
            prev = cur;
            cur = cur->_next;

            old_prev->node_mutex.unlock(); // îáðàòèòå âíèìàíèå, ïîñëå ñäâèãà ÷òî àíëî÷èòñÿ "ñòàðûé" ïðåäûäóùèé ýëåìåíò
            if (cur)
                cur->node_mutex.lock(); // à ïîòîì ëî÷èòñÿ íîâûé òåêóùèé ýëåìåíò
        }
        prev->node_mutex.unlock();
    }

    void show()
    {
        Node* current = _head;
        while (current != nullptr)
        {
            cout << current->_value << " ";
            current = current->_next;
        }
        cout << endl;
    }

};
int main()
{
    FineGrainedQueue FGQ;
    int size = 10;

    for (size_t i = 0; i < size; i++)
    {
        FGQ.push_back(i);
    }
    thread t1([&FGQ]() {FGQ.remove(0); });
    thread t2([&FGQ]() {FGQ.remove(1); });
    thread t3([&FGQ]() {FGQ.remove(2); });
    thread t4([&FGQ]() {FGQ.remove(3); });
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    FGQ.show();

    return 0;
}
