#include "red_black_tree.h"
#include "interval_tree.h"

typedef struct interval_tree_node_info {
  interval_t *interval;
  uint32_t max;
} interval_tree_node_info_t;

struct interval_tree 
{
  rb_red_blk_tree *rb_tree;

  // temp array to store results and avoid constant reallocs
  interval_t **matches;
  int num_matches;
  int max_matches_alloc;
};

int _compKeys(const void *a, const void *b)
{
  if( *(uint32_t*)a > *(uint32_t*)b) return(1);
  if( *(uint32_t*)a < *(uint32_t*)b) return(-1);
  return(0);
}

void _destKey(void *key)
{
  *(uint32_t*)key=0;
  free((uint32_t*)key);
}

void _destInfo(void *info)
{
  ((interval_tree_node_info_t*)info)->interval->start=0;
  ((interval_tree_node_info_t*)info)->interval->end=0;
  ((interval_tree_node_info_t*)info)->interval->data=NULL;

  free(((interval_tree_node_info_t*)info)->interval);
  ((interval_tree_node_info_t*)info)->interval=NULL;

  ((interval_tree_node_info_t*)info)->max=0;
  free((interval_tree_node_info_t*)info);
}

void _printKey(const void *key)
{
  printf("%u",*(uint32_t*)key);
}

void _printInfo(void *info)
{
  printf("%u-%u (max: %u)", 
  	((interval_tree_node_info_t*)info)->interval->start, 
  	((interval_tree_node_info_t*)info)->interval->end, 
  	((interval_tree_node_info_t*)info)->max
  );
}

void _rotationCallback(rb_red_blk_node *rot_node)
{
  // Update max of the affected nodes during tree rotation
  uint32_t max = ((interval_tree_node_info_t*)(rot_node->info))->interval->end;

  if ((rot_node->left->info != NULL)
      && (max < ((interval_tree_node_info_t*)(rot_node->left->info))->max))
    { 
      max = ((interval_tree_node_info_t*)(rot_node->left->info))->max;
    }

  if ((rot_node->right->info != NULL)
      && (max < ((interval_tree_node_info_t*)(rot_node->right->info))->max))
    {
      max = ((interval_tree_node_info_t*)(rot_node->right->info))->max;
    }

  ((interval_tree_node_info_t*)(rot_node->info))->max = max;
}


interval_tree_t *interval_tree_init()
{
  interval_tree_t *this;

  if((this = malloc(sizeof(interval_tree_t))) == NULL)
  {
    return NULL;
  }

  this->rb_tree = RBTreeCreate(_compKeys, _destKey, _destInfo, _printKey, _printInfo, _rotationCallback);

  this->matches = NULL;
  this->num_matches = 0;
  this->max_matches_alloc = 0;

  return this;
}

void interval_tree_free(interval_tree_t *this)
{
  RBTreeDestroy(this->rb_tree);
  this->rb_tree=NULL;

  free(this->matches);
  this->matches=NULL;
  this->max_matches_alloc=0;

  free(this);
}

int interval_tree_add_interval(interval_tree_t *this, const interval_t *interval)
{
  uint32_t *key;
  interval_tree_node_info_t *info;

  if( (key = malloc(sizeof(uint32_t))) == NULL || \
  	  (info = malloc(sizeof(interval_tree_node_info_t))) == NULL || \
  	  (info->interval = malloc(sizeof(interval_t))) == NULL)
    {
      return -1;
    }
  
  *key = interval->start;
  *(info->interval) = *interval;
  info->max=interval->end;

  rb_red_blk_node *node = RBTreeInsert(this->rb_tree, key, info);

  // Adjust all the parents maxes
  while ((node=node->parent) != this->rb_tree->root)
    {
  	  if ( ((interval_tree_node_info_t *)(node->info))->max<info->max)
        {
          ((interval_tree_node_info_t *)(node->info))->max=info->max;
        }
    }

  return 0;
}

int _find(interval_tree_t *tree, rb_red_blk_node *tree_node, const interval_t *interval,
			int (*CmpFunc)(const interval_t*,const interval_t*) )
{

  rb_red_blk_node *nil_node = tree->rb_tree->nil;

  interval_tree_node_info_t *node_info = (interval_tree_node_info_t *)(tree_node->info);

  if ((node_info!=NULL) && (interval->start > node_info->max))
    {
      // Interval is to the right of the rightmost point of any sub-node
      // in this node and all children, there won't be any matches.
      return 0;
    }

  // Search left children
  if (tree_node->left != nil_node)
    {
      if (_find(tree, tree_node->left, interval, CmpFunc) == -1)
        {
          return -1;
        }
    }

  // Check this node
  if ((node_info!=NULL) && CmpFunc(node_info->interval, interval))
    {
      // Match, add it to results
      if (tree->num_matches >= tree->max_matches_alloc)
        {
          // Need to realloc (in batches of 10)
          if ( (tree->matches = realloc(tree->matches, 
          					sizeof(interval_t*) * (tree->max_matches_alloc+10))
          	   ) == NULL) 
            {
              return -1;
            }
          tree->max_matches_alloc+=10;
        }

      tree->matches[tree->num_matches] = node_info->interval;
      tree->num_matches++;
    }

  // If interval is to the left of the start of this node,
  // it can't be in any child to the right.
  if ((tree_node->right != nil_node) && (node_info==NULL || interval->end>=node_info->interval->start))
    {
      // Search right children
      if (_find(tree, tree_node->right, interval, CmpFunc) == -1)
        {
      	  return -1;
        }
    }

  return 0;
}

int _aContainsB(const interval_t *a, const interval_t *b)
{
  if (a->start <= b->start && a->end >= b->end)
    {
    	return 1;
    }
  return 0;
}

int _bContainsA(const interval_t *a, const interval_t *b)
{
  return _aContainsB(b, a);
}

int _touches(const interval_t *a, const interval_t *b)
{
  if (a->start <= b->end && b->start <= a->end)
    {
  	  return 1;
    }
  return 0;
}

interval_t** _getMatches(interval_tree_t *this, const interval_t *interval, 
					int (*CmpFunc)(const interval_t*, const interval_t*), int *num_matches)
{
  this->num_matches=0;
  if (_find(this, this->rb_tree->root, interval, CmpFunc) == -1)
  {
  	// Couldn't malloc
  	*num_matches=-1;
    return NULL;
  }
  *num_matches = this->num_matches;
  return this->matches;	
}


interval_t** getContained(interval_tree_t *this, const interval_t *interval, int *num_matches)
{
  return _getMatches(this, interval, _bContainsA, num_matches);
}

interval_t** getContaining(interval_tree_t *this, const interval_t *interval, int *num_matches)
{
  return _getMatches(this, interval, _aContainsB, num_matches);
}

interval_t** getOverlapping(interval_tree_t *this, const interval_t *interval, int *num_matches)
{
  return _getMatches(this, interval, _touches, num_matches);
}