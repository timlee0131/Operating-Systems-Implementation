#include <list>
#include <vector>

class PRDS_MRU {

	public:

	PRDS_MRU(int count) {}
	void push(int x) {
		helperStructure.push_front(x);
	}
	void remove(int x) {
		helperStructure.remove(x);
	}

	int pop() {
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

int Page_Replacement_MRU(std::vector<int>& pages, int nextpage, PRDS_MRU *p)
{
	bool found = false;
	int i;
	nextpage = abs(nextpage);

	/*
      Check if nextpage is in the pages array, if so return -1
	*/
	for (i = 0; i < pages.size(); i++)
	{
		if (abs(pages[i]) == nextpage)
		{
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
		int to_replace = p->pop();
		for(i = 0; i < pages.size(); i++) {
			if(std::abs(pages[i]) == to_replace)
				break;
		}

		p->push(nextpage);
	} 

	return i;
}
