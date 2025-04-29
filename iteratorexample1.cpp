#include <list>
#include <iterator>
#include <iostream>

using namespace std;

void printList(list<int> &mylist){ // helper function to print the list
    for(int val : mylist){
        cout << val << " ";
    }
    cout << endl;
}

int main(){
    list<int> mylist; // create a doubly linked list of integers

    mylist.push_back(1); // add an element to the end of the list
    mylist.push_front(2); // add an element to the front of the list
    mylist.push_back(3); // add an element to the end of the list

    printList(mylist); // should see <2 1 3>

    cout << "Front: " << mylist.front() << endl; // should see 2
    cout << "Back: " << mylist.back() << endl; // should see 3

    mylist.pop_front(); // remove the first element

    printList(mylist); // should see <1 3>

    mylist.pop_back(); // remove the last element

    printList(mylist); // should see <1>

    /*
    Iterators:

    Since the implmentation of list does not allow for random access, we need to use iterators to traverse the list.

    An iterator is effectively a pointer to the current element in the list. It allows us to move forward and backward in the list without 
    having direct access to node.current, node.next, or node.prev.

    */

    mylist.push_back(2); // repopulating the list
    mylist.push_back(3);
    mylist.push_back(4);
    mylist.push_back(5);

    printList(mylist); // should see <1 2 3 4 5>

    list<int>::iterator it = mylist.begin(); // create an iterator that points to the first element in the list

    cout << "First element: " << *it << endl; // should see 1

    it++; // move the iterator to the next element

    cout << "Second element: " << *it << endl; // should see 2

    it--; // move the iterator back to the previous element

    cout << "First element: " << *it << endl; // should see 1

    it = mylist.end(); // list.end() moves the iterator to the element ***past*** the end of the list (technically out of bounds)
    it--; // move the iterator back to the last element in the list

    cout << "Last element: " << *it << endl; // should see 5

    // Insert puts elements directly before the iterator
    mylist.insert(it, 6); 

    printList(mylist); // should see <1 2 3 4 6 5>

    mylist.insert(it, 7); 
    mylist.insert(it, 8); 

    printList(mylist);  // should see <1 2 3 4 6 7 8 5>

    it = mylist.begin(); // reset the iterator to the beginning of the list

    // Erase removes the element at the iterator
    cout << "current element: " << *it << endl; // should see 1
    it = mylist.erase(it); 
    cout << "current element: " << *it << endl; // should see 2
    printList(mylist); // should see <2 3 4 6 7 8 5>

    // If you want to edit the value of an element at the iterator, you can dereference the iterator and assign a new value
    int element = *it;
    element = 10;
    *it = element;

    printList(mylist); // should see <10 3 4 6 7 8 5>
}
