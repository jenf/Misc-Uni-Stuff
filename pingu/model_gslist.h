#ifndef _model_gslist_h_included_
#define _model_gslist_h_included_

#include <gtk/gtk.h>

/* Some boilerplate GObject defines. 'klass' is used
 *   instead of 'class', because 'class' is a C++ keyword */

#define MODELGSLIST_TYPE_LIST            (model_gslist_get_type ())
#define MODELGSLIST_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MODELGSLIST_TYPE_LIST, ModelGslist))
#define MODELGSLIST_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MODELGSLIST_TYPE_LIST, ModelGslistClass))
#define MODELGSLIST_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MODELGSLIST_TYPE_LIST))
#define MODELGSLIST_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MODELGSLIST_TYPE_LIST))
#define MODELGSLIST_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MODELGSLIST_TYPE_LIST, ModelGslistClass))

/* The data columns that we export via the tree model interface */

typedef struct _ModelGslist       ModelGslist;
typedef struct _ModelGslistClass  ModelGslistClass;



/* ModelGslist: this structure contains everything we need for our
 *             model implementation. You can add extra fields to
 *             this structure, e.g. hashtables to quickly lookup
 *             rows or whatever else you might need, but it is
 *             crucial that 'parent' is the first member of the
 *             structure.                                          */

typedef gchar *(*GetColumn) (int colno,void *data); // Return is assumed to be dynamically created using g_malloc
typedef gint (*GetNumberColumns) ();
typedef GType (*GetColumnType) (gint column);
struct _ModelGslist
{
  GObject          parent;      /* this MUST be the first member */

  GSList	   *list;
  GetColumn	   getcolumn;
  GetNumberColumns getnumbercolumns;
  GetColumnType    gettypes;
  gint             stamp;       /* Random integer to check whether an iter belongs to our model */
};



/* ModelGslistClass: more boilerplate GObject stuff */

struct _ModelGslistClass
{
  GObjectClass parent_class;
};

GSList *model_gslist_get(GtkTreeModel *tree,GtkTreeIter *iter);

GType             model_gslist_get_type (void);

ModelGslist       *model_gslist_new (GetColumn cols,GetNumberColumns nocols,GetColumnType types);

GSList *model_gslist_get_list(ModelGslist *model_gslist);

#endif /* _model_gslist_h_included_ */
