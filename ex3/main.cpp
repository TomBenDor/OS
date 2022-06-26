#include <thread>
#include "queue"
#include "iostream"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <mutex>

std::vector <std::string> split(const std::string &s, char delim) {
    std::vector <std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim)) {
        result.push_back(item);
    }

    return result;
}

class CountingSemaphore {
private:
    int m_value;
    std::mutex m_mutex_s{};
    std::mutex m_delay_s{};
public:
    CountingSemaphore(int startValue) : m_value(startValue) {
        m_delay_s.lock();
    }

    void down() {
        m_mutex_s.lock();
        m_value--;
        if (m_value < 0) {
            m_mutex_s.unlock();
            m_delay_s.lock();
        }

        m_mutex_s.unlock();
    }

    void up() {
        m_mutex_s.lock();
        m_value++;
        if (m_value <= 0) {
            m_delay_s.unlock();
        } else {
            m_mutex_s.unlock();
        }
    }

    int value() const {
        return m_value;
    }
};

class BoundedQueue : public std::queue<std::string> {
private:
    int max_size;
    std::mutex mutex{};
    CountingSemaphore empty_sem, full_sem;
public:
    explicit BoundedQueue(int max_size) : max_size(max_size), mutex(), empty_sem(max_size), full_sem(0) {

    }

    void enqueue(const std::string &s) {
        empty_sem.down();
        mutex.lock();
        push(s);
        mutex.unlock();
        full_sem.up();
    }

    std::string dequeue() {
        if (empty()) {
            return "";
        }
        full_sem.down();
        mutex.lock();
        std::string s = front();
        pop();
        mutex.unlock();
        empty_sem.up();
        return s;
    }
};

class Queue : public std::queue<std::string> {
private:
    std::mutex mutex{};
    CountingSemaphore full;
public:
    Queue() : mutex(), full(0) {

    }

    void enqueue(const std::string &s) {
        mutex.lock();
        push(s);
        mutex.unlock();
        full.up();
    }

    std::string dequeue() {
        if (empty()) {
            return "";
        }
        full.down();
        mutex.lock();
        std::string s = front();
        pop();
        mutex.unlock();
        return s;
    }
};

std::vector <std::unique_ptr<BoundedQueue>> producers_queues;
std::vector <std::unique_ptr<Queue>> co_editors_queues;

void producer(int id, int numberOfProducts) {
    int sports = 0, news = 0, weather = 0;

    for (int i = 0; i < numberOfProducts; ++i) {
        // Pick a random type of product
        int type = rand() % 3;
        // Create a product
        std::string product;
        switch (type) {
            case 0:
                product = "SPORTS " + std::to_string(sports++);
                break;
            case 1:
                product = "NEWS " + std::to_string(news++);
                break;
            case 2:
                product = "WEATHER " + std::to_string(weather++);
                break;
        }
        std::string s = "producer " + std::to_string(id) + " " + product;
        producers_queues[id]->enqueue(s);
    }

    producers_queues[id]->enqueue("DONE");
}

void dispatcher() {
    for (int current_queue = 0, ended = 0;ended != producers_queues.size(); current_queue = (current_queue + 1) % producers_queues.size()) {
        std::string s = producers_queues[current_queue]->dequeue();
        if (s == "")
            continue;
        if (s == "DONE") {
            ended++;
            continue;
        }
        if (s.find("SPORTS") != std::string::npos) {
            co_editors_queues[0]->enqueue(s);
        }
        if (s.find("NEWS") != std::string::npos) {
            co_editors_queues[1]->enqueue(s);
        }
        if (s.find("WEATHER") != std::string::npos) {
            co_editors_queues[2]->enqueue(s);
        }
    }
    for (int i = 0; i < co_editors_queues.size(); ++i) {
        co_editors_queues[i]->enqueue("DONE");
    }
}

void co_editor(int i, BoundedQueue *screen_queue) {
    while (true) {
        std::string s = co_editors_queues[i]->dequeue();
        if (s == "")
            continue;
        if (s == "DONE") {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        screen_queue->enqueue(s);
    }
    screen_queue->enqueue("DONE");
}

int main(int argc, char const *argv[]) {
    std::vector <std::thread> threads;

    std::ifstream t(argv[1]);
    std::string config((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    std::vector <std::string> v = split(config, '\n');
    for (int i = 0; i < v.size() - 1; i += 4) {
        int id = std::stoi(v[i]) - 1;
        int numberOfProducts = std::stoi(v[i + 1]);
        int queue_size = std::stoi(v[i + 2]);
        producers_queues.push_back(std::unique_ptr<BoundedQueue>(new BoundedQueue(queue_size)));
        threads.push_back(std::thread(producer, id, numberOfProducts));
    }

    int co_editors_queues_size = std::stoi(v[v.size() - 1]);


    for (int i = 0; i < 3; ++i) {
        co_editors_queues.push_back(std::unique_ptr<Queue>(new Queue()));
    }

    BoundedQueue screen_queue(co_editors_queues_size);

    threads.push_back(std::thread(dispatcher));
    threads.push_back(std::thread(co_editor, 0, &screen_queue));
    threads.push_back(std::thread(co_editor, 1, &screen_queue));
    threads.push_back(std::thread(co_editor, 2, &screen_queue));

    int ended = 0;
    while (ended != 3) {
        std::string s = screen_queue.dequeue();
        if (s == "")
            continue;
        if (s == "DONE") {
            ended++;
            continue;
        }
        std::cout << s << std::endl;
    }

    for (int i = 0; i < threads.size(); ++i) {
        threads[i].join();
    }
}