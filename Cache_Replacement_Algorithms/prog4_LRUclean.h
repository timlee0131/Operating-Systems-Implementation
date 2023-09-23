#include <vector>
#include <list>
#include <iostream>

/*
	new idea to solve the dirty page issue. instead of checking p, check for whether or not pages[i] is a negative number. if so, it is dirty
*/

class PRDS_LRUclean {

	public:

	PRDS_LRUclean(int count) {}
	void push(int x) {
		helperStructure.push_front(x);
	}
	void remove(int x) {
		helperStructure.remove(x);
	}
	int back() {
		return helperStructure.back();
	}
	int front() {
		return helperStructure.front();
	}
	int pop() {
		int x = helperStructure.back();
		helperStructure.pop_back();
		return x;
	}
	int popFront() {
		int x = helperStructure.front();
		helperStructure.pop_front();
		return x;
	}
	void moveToFront(int x) {
		helperStructure.remove(x);
		helperStructure.push_front(x);
	}

	std::list<int> helperStructure;
};

int Page_Replacement_LRUclean(std::vector<int>& pages, int nextpage, PRDS_LRUclean *p)
{
	bool found = false;
	int i;
	int nextpageParity = nextpage;
	nextpage = abs(nextpage);

	// check if nextpage is in the pages array, if so return -1
	for(i = 0; i < pages.size(); i++) {
		if(abs(pages[i]) == nextpage) {
			found = true;
			p->moveToFront(nextpage);

			return -1;
		}
	}

	if(not found) {
		// check if theres an empty slot, if so return index for that slot
		for(i = 0; i < pages.size(); i++) {
			if(pages[i] == 0) {
				pages[i] = nextpage;
				p->push(nextpage);
				return i;
			}
		}

		// get page to be replaced. find where it is stored in pages vector
		std::list<int>::reverse_iterator rit;
		int j;
		for(rit = p->helperStructure.rbegin(); rit != p->helperStructure.rend(); ++rit) {
			for(i = 0; i < pages.size(); i++) {
				if(abs(pages[i]) == p->back())
					j = i;
				if(abs(pages[i]) == *rit) {
					if(pages[i] > -1) {
						p->remove(abs(pages[i]));
						p->push(nextpage);
						return i;
					}
				}
			}
		}
		int to_replace = p->pop();
		p -> push(nextpage);
		return j;
	}
}
