//
// Created by epat on 11.07.2021.
//

#ifndef GBEMU_EVENT_H
#define GBEMU_EVENT_H
#include <vector>
#include <functional>
#include <type_traits>
#include <memory>
#include <set>



template<class T> class Event {
    static_assert(std::is_integral<T>::value, "has to be integral an type or void");
public:
    void HasDelegates();
    void Invoke();

    void operator += (std::shared_ptr<std::function<void(T)>> func){
        Delegates.insert(func);
    }
    void operator -= (std::shared_ptr<std::function<void(T)>> func){
        //auto pos = Delegates.find(func);
        Delegates.erase(func);
    }

private:
    std::set<std::shared_ptr<std::function<void(T)>>> Delegates;
};





#endif //GBEMU_EVENT_H
