#include <iterator>
#include <vector>
#include <utility>
#include <algorithm>

#include "ADM_default.h"
#include "gui_action.hxx"

using std::vector;
using std::pair;
using std::make_pair;
using std::back_inserter;
using std::transform;
using std::sort;
using std::equal_range;

struct ActionNameNum
{
    const Action num;
    const char * const name;
};

// The getActionName() function assumes that the following array is sorted by
// action number, and further that it can be indexed by action number based on
// the action number of the first entry.

const ActionNameNum action_names [] =
{
#define ACT(_name) { ACT_ ## _name, # _name },
#include "gui_action.names"
#undef ACT
    //   { ACT_INVALID, 0 }
};

const int ACTION_NAME_COUNT = sizeof (action_names) / sizeof (action_names[0]);

// One might think to use a map here, instead of a vector (and in fact I did),
// but Scott Meyers in _Effective STL_ advises (in item 23) that it is more
// efficient to use a sorted vector when we are doing a bunch of insertions
// followed by mostly (in our case exclusively) lookups.

typedef pair <const char *, Action> ActionNamePair;
typedef vector <ActionNamePair> ActionNameVec;
typedef ActionNameVec::iterator ActionNameIter;

class ActionNameCompare
{
public:
    bool operator () (const ActionNamePair & lhs, // for sorting
                      const ActionNamePair & rhs) const
    {
        return doLess (lhs.first, rhs.first);
    }

    bool operator () (const ActionNamePair & lhs, // for lookups 1
                      const ActionNamePair::first_type & rhs) const
    {
        return doLess (lhs.first, rhs);
    }

    bool operator () (const ActionNamePair::first_type & lhs, // for lookups 2
                      const ActionNamePair & rhs) const
    {
        return doLess (lhs, rhs.first);
    }

private:

    bool doLess (const ActionNamePair::first_type & lhs,
                 const ActionNamePair::first_type & rhs) const
    {
        return (strcasecmp (lhs, rhs) < 0);
    }
};

class MakeActionNamePair
{
public:
    ActionNamePair operator () (const ActionNameNum & ann)
    {
        return make_pair (ann.name, ann.num);
    }
};

static ActionNameVec action_name_vec;

void initActionNameVec (void)
{
    action_name_vec.reserve (ACTION_NAME_COUNT);
    transform (action_names, action_names + ACTION_NAME_COUNT,
               back_inserter (action_name_vec), MakeActionNamePair());
    sort (action_name_vec.begin(), action_name_vec.end(),
          ActionNameCompare());
}

Action lookupActionByName (const char * name)
{
    if (action_name_vec.empty())
        initActionNameVec();

    pair <ActionNameIter, ActionNameIter> range
        = equal_range (action_name_vec.begin(),
                       action_name_vec.end(), name,
                       ActionNameCompare());
    if (range.first == range.second)
        return ACT_INVALID;
    else
        return range.first->second;
}

const char * getActionName (Action act)
{
	if(act==ACT_DUMMY) return "ACT_DUMMY";
	if(act>=ACT_SCRIPT_ENGINE_FIRST) return "ACT_SCRIPT_ENGINE";
    if(act>=ACT_CUSTOM_BASE_PY) return "ACT_PY_SCRIPT";
    if(act>=ACT_CUSTOM_BASE_JS) return "ACT_JS_SCRIPT";

    uint32_t index = act - action_names [0].num;
    return (action_names [index].name);
}

void dumpActionNames (const char * filename)
{
    if (action_name_vec.empty())
        initActionNameVec();

    printf ("# Action names (which can be used in %s, "
            "though some should not be):\n", filename);
    ActionNameIter it = action_name_vec.begin();
    while (it != action_name_vec.end())
    {
        printf ("#     %s\n", it->first);
        ++it;
    }
}
