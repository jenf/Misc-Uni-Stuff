#include "model_gslist.h"

/* this code is based around
 * http://scentric.net/tutorial/sec-custom-models.html
 * but edited to make it generic to gslists */

/* boring declarations of local functions */

static void         model_gslist_init            (ModelGslist      *pkg_tree);

static void         model_gslist_class_init      (ModelGslistClass *klass);

static void         model_gslist_tree_model_init (GtkTreeModelIface *iface);

static void         model_gslist_finalize        (GObject           *object);

static GtkTreeModelFlags model_gslist_get_flags  (GtkTreeModel      *tree_model);

static gint         model_gslist_get_n_columns   (GtkTreeModel      *tree_model);

static GType        model_gslist_get_column_type (GtkTreeModel      *tree_model,
                                                 gint               index);

static gboolean     model_gslist_get_iter        (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreePath       *path);

static GtkTreePath *model_gslist_get_path        (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static void         model_gslist_get_value       (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 gint               column,
                                                 GValue            *value);

static gboolean     model_gslist_iter_next       (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gboolean     model_gslist_iter_children   (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *parent);

static gboolean     model_gslist_iter_has_child  (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gint         model_gslist_iter_n_children (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter);

static gboolean     model_gslist_iter_nth_child  (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *parent,
                                                 gint               n);

static gboolean     model_gslist_iter_parent     (GtkTreeModel      *tree_model,
                                                 GtkTreeIter       *iter,
                                                 GtkTreeIter       *child);


static GObjectClass *parent_class = NULL;  /* GObject stuff - nothing to worry about */


/*****************************************************************************
 *
 *  model_gslist_get_type: here we register our new type and its interfaces
 *                        with the type system. If you want to implement
 *                        additional interfaces like GtkTreeSortable, you
 *                        will need to do it here.
 *
 *****************************************************************************/

GType
model_gslist_get_type (void)
{
  static GType model_gslist_type = 0;

  if (model_gslist_type)
    return model_gslist_type;

  /* Some boilerplate type registration stuff */
  if (1)
  {
    static const GTypeInfo model_gslist_info =
    {
      sizeof (ModelGslistClass),
      NULL,                                         /* base_init */
      NULL,                                         /* base_finalize */
      (GClassInitFunc) model_gslist_class_init,
      NULL,                                         /* class finalize */
      NULL,                                         /* class_data */
      sizeof (ModelGslist),
      0,                                           /* n_preallocs */
      (GInstanceInitFunc) model_gslist_init
    };

    model_gslist_type = g_type_register_static (G_TYPE_OBJECT, "ModelGslist",
                                               &model_gslist_info, (GTypeFlags)0);
  }

  /* Here we register our GtkTreeModel interface with the type system */
  if (1)
  {
    static const GInterfaceInfo tree_model_info =
    {
      (GInterfaceInitFunc) model_gslist_tree_model_init,
      NULL,
      NULL
    };

    g_type_add_interface_static (model_gslist_type, GTK_TYPE_TREE_MODEL, &tree_model_info);
  }

  return model_gslist_type;
}


/*****************************************************************************
 *
 *  model_gslist_class_init: more boilerplate GObject/GType stuff.
 *                          Init callback for the type system,
 *                          called once when our new class is created.
 *
 *****************************************************************************/

static void
model_gslist_class_init (ModelGslistClass *klass)
{
  GObjectClass *object_class;

  parent_class = (GObjectClass*) g_type_class_peek_parent (klass);
  object_class = (GObjectClass*) klass;
  

  object_class->finalize = model_gslist_finalize;
}

/*****************************************************************************
 *
 *  model_gslist_tree_model_init: init callback for the interface registration
 *                               in model_gslist_get_type. Here we override
 *                               the GtkTreeModel interface functions that
 *                               we implement.
 *
 *****************************************************************************/

static void
model_gslist_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags       = model_gslist_get_flags;
  iface->get_n_columns   = model_gslist_get_n_columns;
  iface->get_column_type = model_gslist_get_column_type;
  iface->get_iter        = model_gslist_get_iter;
  iface->get_path        = model_gslist_get_path;
  iface->get_value       = model_gslist_get_value;
  iface->iter_next       = model_gslist_iter_next;
  iface->iter_children   = model_gslist_iter_children;
  iface->iter_has_child  = model_gslist_iter_has_child;
  iface->iter_n_children = model_gslist_iter_n_children;
  iface->iter_nth_child  = model_gslist_iter_nth_child;
  iface->iter_parent     = model_gslist_iter_parent;
}


/*****************************************************************************
 *
 *  model_gslist_init: this is called everytime a new file list object
 *                    instance is created (we do that in model_gslist_new).
 *                    Initialise the list structure's fields here.
 *
 *****************************************************************************/

static void
model_gslist_init (ModelGslist *model_gslist)
{

  model_gslist->stamp = g_random_int();  /* Random int to check whether an iter belongs to our model */

}


/*****************************************************************************
 *
 *  model_gslist_finalize: this is called just before a file list is
 *                        destroyed. Free dynamically allocated memory here.
 *
 *****************************************************************************/

static void
model_gslist_finalize (GObject *object)
{
/*  ModelGslist *model_gslist = MODELGSLIST_LIST(object); */

  /* free all records and free all memory used by the list */
  #warning IMPLEMENT

  /* must chain up - finalize parent */
  (* parent_class->finalize) (object);
}


/*****************************************************************************
 *
 *  model_gslist_get_flags: tells the rest of the world whether our tree model
 *                         has any special characteristics. In our case,
 *                         we have a list model (instead of a tree), and each
 *                         tree iter is valid as long as the row in question
 *                         exists, as it only contains a pointer to our struct.
 *
 *****************************************************************************/

static GtkTreeModelFlags
model_gslist_get_flags (GtkTreeModel *tree_model)
{
  g_return_val_if_fail (MODELGSLIST_IS_LIST(tree_model), (GtkTreeModelFlags)0);

  return (GTK_TREE_MODEL_LIST_ONLY | GTK_TREE_MODEL_ITERS_PERSIST);
}


/*****************************************************************************
 *
 *  model_gslist_get_n_columns: tells the rest of the world how many data
 *                             columns we export via the tree model interface
 *
 *****************************************************************************/

static gint
model_gslist_get_n_columns (GtkTreeModel *tree_model)
{
  ModelGslist    *model_gslist;
	                                                                                    
  g_assert(MODELGSLIST_IS_LIST(tree_model));
  model_gslist = MODELGSLIST_LIST(tree_model);
  return model_gslist->getnumbercolumns(NULL);
}


/*****************************************************************************
 *
 *  model_gslist_get_column_type: tells the rest of the world which type of
 *                               data an exported model column contains
 *
 *****************************************************************************/

static GType
model_gslist_get_column_type (GtkTreeModel *tree_model,
                             gint          index)
{
  ModelGslist *model_gslist;
  model_gslist=MODELGSLIST_LIST(tree_model);
  return model_gslist->gettypes(index);
}


/*****************************************************************************
 *
 *  model_gslist_get_iter: converts a tree path (physical position) into a
 *                        tree iter structure (the content of the iter
 *                        fields will only be used internally by our model).
 *                        We simply store a pointer to our FileRecord
 *                        structure that represents that row in the tree iter.
 *
 *****************************************************************************/

static gboolean
model_gslist_get_iter (GtkTreeModel *tree_model,
                      GtkTreeIter  *iter,
                      GtkTreePath  *path)
{
  GSList *record;
  ModelGslist    *model_gslist;
  gint          *indices, n, depth;

  g_assert(MODELGSLIST_IS_LIST(tree_model));
  g_assert(path!=NULL);

  model_gslist = MODELGSLIST_LIST(tree_model);
  
  indices = gtk_tree_path_get_indices(path);
  depth   = gtk_tree_path_get_depth(path);

  /* we do not allow children */
  g_assert(depth == 1); /* depth 1 = top level; a list only has top level nodes and no children */

  n = indices[0]; /* the n-th top level row */

  if ( n >= g_slist_length(model_gslist->list) || n < 0 )
    return FALSE;

  record=g_slist_nth(model_gslist->list,n);
  
  /* We simply store a pointer to our file record in the iter */
  iter->stamp      = model_gslist->stamp;
  iter->user_data  = record;
  iter->user_data2 = NULL;   /* unused */
  iter->user_data3 = NULL;   /* unused */

  return TRUE;
}


/*****************************************************************************
 *
 *  model_gslist_get_path: converts a tree iter into a tree path (ie. the
 *                        physical position of that row in the list).
 *
 *****************************************************************************/

static GtkTreePath *
model_gslist_get_path (GtkTreeModel *tree_model,
                      GtkTreeIter  *iter)
{
  GtkTreePath  *path;
  ModelGslist   *model_gslist;
  GSList *record;
  g_return_val_if_fail (MODELGSLIST_IS_LIST(tree_model), NULL);
  g_return_val_if_fail (iter != NULL,               NULL);
  g_return_val_if_fail (iter->user_data != NULL,    NULL);

  model_gslist = MODELGSLIST_LIST(tree_model);

  record = (GSList*) iter->user_data;
  g_warning("model_gslist_get_path");
  path = gtk_tree_path_new();
  gtk_tree_path_append_index(path, g_slist_index(model_gslist->list,record));

  return path;
}

GSList *model_gslist_get(GtkTreeModel *tree,GtkTreeIter *iter)
{
	ModelGslist *model_gslist;
	GSList *record;
	record=(GSList *) iter->user_data;
	return record;
}

/*****************************************************************************
 *
 *  model_gslist_get_value: Returns a row's exported data columns
 *                         (_get_value is what gtk_tree_model_get uses)
 *
 *****************************************************************************/

static void
model_gslist_get_value (GtkTreeModel *tree_model,
                       GtkTreeIter  *iter,
                       gint          column,
                       GValue       *value)
{
  ModelGslist    *model_gslist;
  GSList *record;
  gchar *m;
  g_return_if_fail (MODELGSLIST_IS_LIST (tree_model));
  g_return_if_fail (iter != NULL);
  g_return_if_fail (column <= 3);

  g_value_init (value, G_TYPE_STRING);

  model_gslist = MODELGSLIST_LIST(tree_model);
  record = (GSList*) iter->user_data;

  g_return_if_fail ( record != NULL );
  m=model_gslist->getcolumn(column,record->data);
  g_value_set_string(value,m);
  g_free(m);
}


/*****************************************************************************
 *
 *  model_gslist_iter_next: Takes an iter structure and sets it to point
 *                         to the next row.
 *
 *****************************************************************************/

static gboolean
model_gslist_iter_next (GtkTreeModel  *tree_model,
                       GtkTreeIter   *iter)
{
  GSList  *record, *nextrecord;
  ModelGslist    *model_gslist;

  g_return_val_if_fail (MODELGSLIST_IS_LIST (tree_model), FALSE);

  if (iter == NULL || iter->user_data == NULL)
    return FALSE;

  model_gslist = MODELGSLIST_LIST(tree_model);
  record = (GSList *) iter->user_data;

  // Fix this
  /* Is this the last record in the list? */
//  if ((record->pos + 1) >= model_gslist->num_rows)
//    return FALSE;

  nextrecord = record->next;

  if (nextrecord==NULL) return FALSE;
  g_assert ( nextrecord != NULL );

  iter->stamp     = model_gslist->stamp;
  iter->user_data = nextrecord;

  return TRUE;
}


/*****************************************************************************
 *
 *  model_gslist_iter_children: Returns TRUE or FALSE depending on whether
 *                             the row specified by 'parent' has any children.
 *                             If it has children, then 'iter' is set to
 *                             point to the first child. Special case: if
 *                             'parent' is NULL, then the first top-level
 *                             row should be returned if it exists.
 *
 *****************************************************************************/

static gboolean
model_gslist_iter_children (GtkTreeModel *tree_model,
                           GtkTreeIter  *iter,
                           GtkTreeIter  *parent)
{
  ModelGslist  *model_gslist;

  g_return_val_if_fail (parent == NULL || parent->user_data != NULL, FALSE);

  /* this is a list, nodes have no children */
  if (parent)
    return FALSE;

  printf("model_gslist_iter_children");
  /* parent == NULL is a special case; we need to return the first top-level row */

  g_return_val_if_fail (MODELGSLIST_IS_LIST (tree_model), FALSE);

  model_gslist = MODELGSLIST_LIST(tree_model);

  /* No rows => no first row */
  if (model_gslist->list ==NULL)
    return FALSE;

  /* Set iter to first item in list */
  iter->stamp     = model_gslist->stamp;
  iter->user_data = model_gslist->list;

  return TRUE;
}


/*****************************************************************************
 *
 *  model_gslist_iter_has_child: Returns TRUE or FALSE depending on whether
 *                              the row specified by 'iter' has any children.
 *                              We only have a list and thus no children.
 *
 *****************************************************************************/

static gboolean
model_gslist_iter_has_child (GtkTreeModel *tree_model,
                            GtkTreeIter  *iter)
{
  return FALSE;
}


/*****************************************************************************
 *
 *  model_gslist_iter_n_children: Returns the number of children the row
 *                               specified by 'iter' has. This is usually 0,
 *                               as we only have a list and thus do not have
 *                               any children to any rows. A special case is
 *                               when 'iter' is NULL, in which case we need
 *                               to return the number of top-level nodes,
 *                               ie. the number of rows in our list.
 *
 *****************************************************************************/

static gint
model_gslist_iter_n_children (GtkTreeModel *tree_model,
                             GtkTreeIter  *iter)
{
  ModelGslist  *model_gslist;

  g_return_val_if_fail (MODELGSLIST_IS_LIST (tree_model), -1);
  g_return_val_if_fail (iter == NULL || iter->user_data != NULL, FALSE);

  model_gslist = MODELGSLIST_LIST(tree_model);

  /* special case: if iter == NULL, return number of top-level rows */
  if (!iter)
    return g_slist_length(model_gslist->list);

  return 0; /* otherwise, this is easy again for a list */
}


/*****************************************************************************
 *
 *  model_gslist_iter_nth_child: If the row specified by 'parent' has any
 *                              children, set 'iter' to the n-th child and
 *                              return TRUE if it exists, otherwise FALSE.
 *                              A special case is when 'parent' is NULL, in
 *                              which case we need to set 'iter' to the n-th
 *                              row if it exists.
 *
 *****************************************************************************/

static gboolean
model_gslist_iter_nth_child (GtkTreeModel *tree_model,
                            GtkTreeIter  *iter,
                            GtkTreeIter  *parent,
                            gint          n)
{
  GSList  *record;
  ModelGslist    *model_gslist;

  g_return_val_if_fail (MODELGSLIST_IS_LIST (tree_model), FALSE);

  model_gslist = MODELGSLIST_LIST(tree_model);

  /* a list has only top-level rows */
  if(parent)
    return FALSE;

  /* special case: if parent == NULL, set iter to n-th top-level row */

  if( n >= g_slist_length(model_gslist->list) )
    return FALSE;

  record = g_slist_nth(model_gslist->list,n);

  g_assert( record != NULL );

  iter->stamp = model_gslist->stamp;
  iter->user_data = record;

  return TRUE;
}


/*****************************************************************************
 *
 *  model_gslist_iter_parent: Point 'iter' to the parent node of 'child'. As
 *                           we have a list and thus no children and no
 *                           parents of children, we can just return FALSE.
 *
 *****************************************************************************/

static gboolean
model_gslist_iter_parent (GtkTreeModel *tree_model,
                         GtkTreeIter  *iter,
                         GtkTreeIter  *child)
{
  return FALSE;
}


/*****************************************************************************
 *
 *  model_gslist_new:  This is what you use in your own code to create a
 *                    new file list tree model for you to use.
 *
 *****************************************************************************/

ModelGslist *
model_gslist_new (GetColumn cols,GetNumberColumns nocols,GetColumnType types)
{
  ModelGslist *newfilelist;

  newfilelist = (ModelGslist*) g_object_new (MODELGSLIST_TYPE_LIST, NULL);

  g_assert( newfilelist != NULL );
  newfilelist->getcolumn=cols;
  newfilelist->getnumbercolumns=nocols;
  newfilelist->gettypes=types;
  newfilelist->list=NULL;
  return newfilelist;
}


/*****************************************************************************
 *
 *  model_gslist_append_record:  Empty lists are boring. This function can
 *                              be used in your own code to add rows to the
 *                              list. Note how we emit the "row-inserted"
 *                              signal after we have appended the row
 *                              internally, so the tree view and other
 *                              interested objects know about the new row.
 *
 *****************************************************************************/

GSList *model_gslist_get_list(ModelGslist *model)
{
	return model->list;
}

GSList *
model_gslist_delete_record (ModelGslist   *model_gslist,void *item)
{
  GtkTreePath  *path;
  gulong        newsize;
  guint         pos;

  g_return_if_fail (MODELGSLIST_IS_LIST(model_gslist));

  /* inform the tree view and other interested objects
   *  (e.g. tree row references) that we have inserted
   *  a new row, and where it was inserted */
  //model_gslist->list=list;
  path = gtk_tree_path_new();
  pos=g_slist_index(model_gslist->list,item);
  gtk_tree_path_append_index(path,pos);

  model_gslist->list=g_slist_remove(model_gslist->list,item);
  gtk_tree_model_row_deleted(GTK_TREE_MODEL(model_gslist), path);

  gtk_tree_path_free(path);
  return model_gslist->list;
}

void
model_gslist_new_record (ModelGslist   *model_gslist,GSList *list)
{
  GtkTreeIter   iter;
  GtkTreePath  *path;
  gulong        newsize;
  guint         pos;

  g_return_if_fail (MODELGSLIST_IS_LIST(model_gslist));

  /* inform the tree view and other interested objects
   *  (e.g. tree row references) that we have inserted
   *  a new row, and where it was inserted */
  model_gslist->list=list;
  printf("%i\n",g_slist_length(list));
  path = gtk_tree_path_new();
  gtk_tree_path_append_index(path,0);
  model_gslist_get_iter(GTK_TREE_MODEL(model_gslist), &iter, path);

  gtk_tree_model_row_inserted(GTK_TREE_MODEL(model_gslist), path, &iter);

  gtk_tree_path_free(path);
}
