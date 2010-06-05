#include <gtk/gtk.h>
#include <netinet/in.h>
#include <signal.h>
#include "model_gslist.h"
int     been_through = 0;
GtkWidget *notebook, *window;

static gchar *get_reply_column(int colno, void *k)
{
	// Called when the socket list thing is done.
	char    gee[256];
	gchar  *reply;
	switch (colno)
	{
	case 0:
		fillin_host_string(gee, 256, k);
		reply = g_strdup(gee);
		break;
	default:
		reply = g_strdup_printf("Bar");
	}
	return reply;
}


static int get_reply_no_columns(void *k)
{
	// Called when the socket list thing is done.
	return 1;
}

static GType get_reply_column_type(gint column)
{
	// Called when the socket list thing is done.
	return G_TYPE_STRING;		// Always a string.
}

GtkWidget *dialog, *entry;
void process_connect_window()
{
	// Read the IP Address the user has said
	gchar  *moo;
	moo = gtk_entry_get_text(GTK_ENTRY(entry));
	do_connection(moo);
	gtk_widget_hide(dialog);
}

char *ask_new_server()
{
	// Show the IP Address dialog
	gchar  *moo, *bar;
	gdk_threads_enter();
	gtk_widget_show_all(dialog);
	gdk_threads_leave();
}


void on_closedown()
{
	// This tends to SEGFAULT
	if (been_through != 0)
		return;
	if (gtk_main_level() != 0)
		gtk_main_quit();
	closedown(1);
	exit(1);
	been_through = 1;
}

static void clicked_on_search(GtkButton * k, GtkWidget * label)
{
	// Call back when we search.
	const gchar *m;
	m = gtk_entry_get_text(GTK_ENTRY(label));
	searchfile(m);
}

static void clicked_filesearch_download(GtkWidget * w, GtkWidget * view)
{
	// When a user clicks on the file search button..
	GtkTreeSelection *sel;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GSList *list;
	sel = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
	if (gtk_tree_selection_get_selected(sel, &model, &iter))
	{
		list = model_gslist_get(model, &iter);
		get_reply_download_file(list->data);
	}
	else
	{
		// We really should grey the widget instead.
		printf("Nothing selected\n");
	}
}

static inline void add_column(GtkCellRenderer * renderer, GtkTreeView * view,
							  gchar * name, gint column)
{
	// Add a column to the list
	GtkTreeViewColumn *col;
	col = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(col, renderer, TRUE);
	gtk_tree_view_column_add_attribute(col, renderer, "text", column);
	gtk_tree_view_column_set_title(col, name);
	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
}

void gui_set_searchtimeout(GtkWidget * widget)
{
	// Set the search to timeout.
	GList  *l;
	gchar  *bar;
	const gchar *moo;
	gdk_threads_enter();
	moo = gtk_label_get_text(GTK_LABEL(widget));
	bar = g_strdup_printf("Finished %s", moo);
	// I can never remember if your suppost to free it.
	gtk_label_set_text(GTK_LABEL(widget), bar);
	gdk_threads_leave();
}

GtkWidget *notebook_toolbar(GtkWidget * title, gchar * stock,
				GCallback onclose, void *callvalues)
{
	GtkWidget *icon, *hbox, *label;
	// This creates a nice widget with a close button and text
	icon = gtk_image_new_from_stock(stock, GTK_ICON_SIZE_MENU);
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(hbox), icon);
	gtk_container_add(GTK_CONTAINER(hbox), title);
	label = gtk_button_new();
	icon = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_button_set_relief(GTK_BUTTON(label), GTK_RELIEF_NONE);
	gtk_container_set_border_width(GTK_CONTAINER(label), 0);
	gtk_container_add(GTK_CONTAINER(label), icon);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	g_signal_connect(G_OBJECT(label), "clicked", onclose, callvalues);
	gtk_widget_show_all(hbox);
	return hbox;
}

void filename_search_close(GtkWidget * button, uint16_t req)
{
	GtkWidget *widget;
	// Call back.
	filesearch_unallocate_request(req);
}

void filename_search_close_freewidgets(GtkWidget * widget)
{
	gint    k;
	// Called to remove a search tab
	k = gtk_notebook_page_num(GTK_NOTEBOOK(notebook), widget);
	printf("%i\n", k);
	gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), k);
}

ModelGslist *servermodel;
void make_servers_window()
{
	GtkWidget *view, *label, *box, *obox;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *select;
	GtkWidget *scroll;
	// Urgh, glib data structures are really horrible. And not thread safe.
	servermodel = model_gslist_new((GetColumn) get_reply_column,
					(GetNumberColumns) get_reply_no_columns,
					(GetColumnType) get_reply_column_type);

	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(servermodel));
	scroll =
		gtk_scrolled_window_new(gtk_tree_view_get_hadjustment
					(GTK_TREE_VIEW(view)),
					gtk_tree_view_get_vadjustment(GTK_TREE_VIEW
					  (view)));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
				GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	printf("make_servers_window\n");
	renderer = gtk_cell_renderer_text_new();
	label = gtk_label_new("Downstream Servers");
	add_column(renderer, GTK_TREE_VIEW(view), "Host", 0);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), view, label);
	gtk_widget_show_all(view);
}

GtkWidget *filename_search_begin(char *filename, ModelGslist * l,
				 uint16_t req)
{
	GtkWidget *view, *label, *box, *obox;
	GtkTreeViewColumn *col;
	GtkCellRenderer *renderer;
	GtkTreeSelection *select;
	GtkWidget *scroll, *label_ret;
	gchar  *tmp;
	view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(l));
	scroll =
		gtk_scrolled_window_new(gtk_tree_view_get_hadjustment
			(GTK_TREE_VIEW(view)),
			gtk_tree_view_get_vadjustment(GTK_TREE_VIEW
			(view)));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),							   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	printf("filename_search_begin\n");
	renderer = gtk_cell_renderer_text_new();

	add_column(renderer, GTK_TREE_VIEW(view), "Filename", 0);
	add_column(renderer, GTK_TREE_VIEW(view), "MD5Sum", 1);
	add_column(renderer, GTK_TREE_VIEW(view), "File size", 2);
	add_column(renderer, GTK_TREE_VIEW(view), "Host", 3);

	// Add a vbox to the notebook
	box = gtk_vbox_new(FALSE, 5);
	// Notebook tab
	tmp = g_strdup_printf("Searching for %s", filename);
	label_ret = gtk_label_new(tmp);
	label =
		notebook_toolbar(label_ret, "gtk-find",
						 (GCallback) filename_search_close,
						 (void *) (int) req);
	g_free(tmp);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), box, label);

	gtk_container_add(GTK_CONTAINER(scroll), view);
	gtk_box_pack_start(GTK_BOX(box), scroll, TRUE, TRUE, 5);
	obox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(box), obox, FALSE, FALSE, 5);
	label = gtk_button_new_with_label("Download");
	gtk_container_add(GTK_CONTAINER(obox), label);

	printf("notebook:%i\n", gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook)));
	g_signal_connect(label, "clicked",
					 (GCallback) clicked_filesearch_download, view);
	gtk_widget_show_all(notebook);
	return label_ret;
}

static guint context;
static GtkWidget *statusbar;
void add_status_message(gchar * text)
{
//  gtk_statusbar_push(GTK_STATUSBAR(statusbar),context,text);
}

int main(int argc, char **argv)
{
	GtkWidget *label, *widget, *vbox, *hbox;
	g_thread_init(NULL);
	gdk_threads_init();
	gtk_init(&argc, &argv);
	if (argc != 2)
	{
		printf("Incorrect number of arguments\n" "pingu <directory>");
		exit(1);
	}
	// This isn't nice.
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 400);
	gtk_window_set_title(GTK_WINDOW(window), "Pingu");
	g_signal_connect(window, "delete_event", on_closedown, NULL);
	notebook = gtk_notebook_new();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
	widget = gtk_label_new("Main");
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, widget);
	hbox = gtk_hbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 5);
	label = gtk_label_new("Search for Filename:");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);
	label = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 5);

	widget = gtk_button_new_from_stock("gtk-find");
	g_signal_connect(widget, "clicked", (GCallback) clicked_on_search, label);
	gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 5);

	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_add(GTK_CONTAINER(vbox), notebook);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	dialog = gtk_message_dialog_new(window,
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_OK,
				"Please enter an IP Address of a servlet to contact\nDo not put localhost");
	entry = gtk_entry_new();
	g_signal_connect(GTK_OBJECT(dialog), "response",
					 G_CALLBACK(process_connect_window), entry);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), entry);
	gtk_widget_show_all(window);
	make_servers_window();
	request_init();
	md5_init(argv[1]);
	download_init();
	setup_sockets();
	signal(SIGINT, on_closedown);
	signal(SIGINT, on_closedown);
	gtk_main();
	raise(SIGINT);
}
