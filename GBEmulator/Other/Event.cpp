//
// Created by epat on 11.07.2021.
//

#include "Event.h"

template<class T>
inline void Event<T>::HasDelegates() {
    return !Delegates.empty();
}

template<class T>
void Event<T>::Invoke() {
    if(!HasDelegates())
        return;

    for(auto delegate : Delegates){
        delegate();
    }
}




