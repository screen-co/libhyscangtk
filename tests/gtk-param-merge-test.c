#include <hyscan-gtk-param-merge.h>
#include <hyscan-param-controller.h>
#include <hyscan-data-schema-builder.h>

#define KEY_SALARY "/salary"
#define KEY_NAME   "/name"
#define KEY_REWARD "/reward"

enum {
  COLUMN_STRING,
  COLUMN_INT,
  COLUMN_BOOLEAN,
  COLUMN_PARAM,
  N_COLUMNS
};

static HyScanDataSchema *schema;
static GtkListStore *list_store;
static GList *iter_list;

/* Создаёт схему данных. */
static void
build_schema (void)
{
  HyScanDataSchemaBuilder *builder;

  builder = hyscan_data_schema_builder_new ("merge-test");
  hyscan_data_schema_builder_key_string_create (builder, KEY_NAME, "Сотрудник", NULL, NULL);
  hyscan_data_schema_builder_key_integer_create (builder, KEY_SALARY, "Зарплата", NULL, 20);
  hyscan_data_schema_builder_key_integer_range (builder, KEY_SALARY, 1, 100, 1);
  hyscan_data_schema_builder_key_boolean_create (builder, KEY_REWARD, "Премировать", NULL, TRUE);

  schema = hyscan_data_schema_builder_get_schema (builder);

  g_object_unref (builder);
}

/* Обработчик считывания параметров. */
static GVariant *
controller_get (const gchar *name,
                gpointer     user_data)
{
  GtkTreeIter *iter = user_data;

  if (g_strcmp0 (name, KEY_NAME) == 0)
    {
      gchar *value;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, COLUMN_STRING, &value, -1);
      return g_variant_new_take_string (value);
    }
  else if (g_strcmp0 (name, KEY_SALARY) == 0)
    {
      gint64 value;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, COLUMN_INT, &value, -1);
      return g_variant_new_int64 (value);
    }
  else if (g_strcmp0 (name, KEY_REWARD) == 0)
    {
      gboolean value;
      gtk_tree_model_get (GTK_TREE_MODEL (list_store), iter, COLUMN_BOOLEAN, &value, -1);
      return g_variant_new_boolean (value);
    }

  return NULL;
}

/* Обработчик установки параметров. */
static gboolean
controller_set (const gchar *name,
                GVariant    *value,
                gpointer     user_data)
{
  GtkTreeIter *iter = user_data;

  if (g_strcmp0 (name, KEY_NAME) == 0)
    gtk_list_store_set (list_store, iter, COLUMN_STRING, g_variant_get_string (value, NULL), -1);

  else if (g_strcmp0 (name, KEY_SALARY) == 0)
    gtk_list_store_set (list_store, iter, COLUMN_INT, g_variant_get_int64 (value), -1);

  else if (g_strcmp0 (name, KEY_REWARD) == 0)
    gtk_list_store_set (list_store, iter, COLUMN_BOOLEAN, g_variant_get_boolean (value), -1);

  else
    return FALSE;

  return TRUE;
}

/* Добавляет строку в модель списка. */
static void
list_store_append (const gchar  *name,
                   gint          salary,
                   gboolean      reward)
{
  GtkTreeIter iter, *iter_copy;
  HyScanParamController *param;

  gtk_list_store_append (list_store, &iter);

  /* Обработчик параметров строки - пишет и читает напрямую из GtkListStore. */
  param = hyscan_param_controller_new (NULL);

  iter_copy = gtk_tree_iter_copy (&iter);
  iter_list = g_list_append (iter_list, iter_copy);

  hyscan_param_controller_set_schema (param, schema);
  hyscan_param_controller_add_user (param, KEY_NAME,   controller_set, controller_get, iter_copy);
  hyscan_param_controller_add_user (param, KEY_SALARY, controller_set, controller_get, iter_copy);
  hyscan_param_controller_add_user (param, KEY_REWARD, controller_set, controller_get, iter_copy);

  gtk_list_store_set (list_store, &iter,
                      COLUMN_STRING, name,
                      COLUMN_INT, salary,
                      COLUMN_BOOLEAN, reward,
                      COLUMN_PARAM, param,
                      -1);
}

/* Создаёт виджет списка. */
static GtkWidget *
tree_view_new (void)
{
  GtkWidget *tree_view;

  list_store = gtk_list_store_new (N_COLUMNS,
                                   G_TYPE_STRING,
                                   G_TYPE_INT64,
                                   G_TYPE_BOOLEAN,
                                   HYSCAN_TYPE_PARAM_CONTROLLER);

  list_store_append ("Иванов",  20, TRUE);
  list_store_append ("Петров",  30, TRUE);
  list_store_append ("Сидоров", 30, TRUE);
  list_store_append ("Петров",  40, TRUE);

  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));

  gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), GTK_SELECTION_MULTIPLE);

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
    gtk_tree_view_column_new_with_attributes ("Сотрудник", gtk_cell_renderer_text_new (), "text", COLUMN_STRING, NULL));

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
    gtk_tree_view_column_new_with_attributes ("Зарплата", gtk_cell_renderer_text_new (), "text", COLUMN_INT, NULL));

  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
    gtk_tree_view_column_new_with_attributes ("Премировать", gtk_cell_renderer_text_new (), "text", COLUMN_BOOLEAN, NULL));

  return tree_view;
}

/* Пересоздаёт виджет редактирования параметров. */
static void
update_param_widget (GtkTreeSelection *selection,
                     gpointer          user_data)
{
  GList *list, *link;
  HyScanParamMerge *merge;
  GtkWidget *widget, *action_bar, *btn_apply;
  GtkWidget *param_box = user_data;

  /* Удаляем из контейнера, что там было раньше. */
  list = gtk_container_get_children (GTK_CONTAINER (param_box));
  for (link = list; link != NULL; link = link->next)
    gtk_container_remove (GTK_CONTAINER (param_box), GTK_WIDGET (link->data));
  g_list_free (list);

  list = gtk_tree_selection_get_selected_rows (selection, NULL);
  if (list == NULL)
    {
      gtk_box_set_center_widget (GTK_BOX (param_box), gtk_label_new ("Выберите строки для редактирования"));
      gtk_widget_show_all (param_box);
      return;
    }

  /* Добавляем в HyScanParamMerge обработчики выбранных строк. */
  merge = hyscan_param_merge_new ();
  for (link = list; link != NULL; link = link->next)
    {
      GtkTreePath *path = link->data;
      HyScanParam *param;
      GtkTreeIter iter;

      gtk_tree_model_get_iter (GTK_TREE_MODEL (list_store), &iter, path);
      gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter, COLUMN_PARAM, &param, -1);
      hyscan_param_merge_add (merge, param);

      g_object_unref (param);
    }
  hyscan_param_merge_bind (merge);

  /* Добавляем виджет в контейнер. */
  widget = hyscan_gtk_param_merge_new_full (merge, "/", FALSE);
  action_bar = gtk_action_bar_new ();

  btn_apply = gtk_button_new_with_label ("Применить");
  g_signal_connect_swapped (btn_apply, "clicked", G_CALLBACK (hyscan_gtk_param_apply), widget);
  gtk_action_bar_pack_end (GTK_ACTION_BAR (action_bar), btn_apply);

  gtk_box_pack_start (GTK_BOX (param_box), widget, TRUE, TRUE, 0);
  gtk_box_pack_end (GTK_BOX (param_box), action_bar, FALSE, TRUE, 0);
  gtk_widget_show_all (param_box);

  g_object_unref (merge);
  g_list_free_full (list, (GDestroyNotify) gtk_tree_path_free);
}

int
main (int    argc,
      char **argv)
{
  GtkWidget *window, *box, *tree, *param_box;

  gtk_init (&argc, &argv);

  build_schema ();

  /* Контейнер для редактора параметров. */
  param_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);

  /* Список объектов для редактирования. */
  tree = tree_view_new ();
  g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree)), "changed",
                    G_CALLBACK (update_param_widget), param_box);
  update_param_widget (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree)), param_box);

  /* Компонуем интерфейс. */
  box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  g_object_set (box, "margin", 12, NULL);
  gtk_box_pack_start (GTK_BOX (box), tree, FALSE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (box), param_box, TRUE, TRUE, 0);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size (GTK_WINDOW (window), 720, 360);
  gtk_container_add (GTK_CONTAINER (window), box);
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (window);

  gtk_main ();

  g_list_free_full (iter_list, (GDestroyNotify) gtk_tree_iter_free);
  g_object_unref (schema);

  return 0;
}
