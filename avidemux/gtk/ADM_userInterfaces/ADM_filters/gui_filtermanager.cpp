/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "ADM_toolkitGtk.h"
#include "ADM_default.h"
#include "DIA_coreToolkit.h"
#include "DIA_fileSel.h"
#include "GUI_glade.h"
#include "ADM_filterCategory.h"
#include "ADM_videoFilterApi.h"
#include "ADM_videoFilters.h"
#include "ADM_coreVideoFilter.h"
#include "ADM_coreVideoFilterFunc.h"
#include "ADM_edit.hxx"

#define nb_active_filter ADM_vf_getSize()

static void displayFamily(GtkTreeView* tree, gpointer data);
static void treeResize(GtkWidget* tree, GtkAllocation* allocation, gpointer renderer);
static void addFilter(GtkButton*, gpointer);
static void configureFilter(GtkButton*, gpointer);
static void previewFilter(GtkButton*, gpointer);
static void saveFilters(GtkButton*, gpointer);
static void loadFilters(GtkButton*, gpointer);
static void filterUp(GtkButton*, gpointer);
static void filterDown(GtkButton*, gpointer);
static void filterRemove(GtkButton*, gpointer);
static void availableDoubleClick(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
static void activeDoubleClick(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer);
static void buildActiveFilterList();
static GtkWidget* storeFamilies;
static GtkWidget* treeAvailable;
static GtkWidget* storeAvailable;
static GtkWidget* treeActive;
static GtkWidget* storeActive;

extern ADM_Composer *video_body;

/**
    \fn GUI_handleVFilter
*/
int GUI_handleVFilter()
{
    ADM_info("Entering video filter\n");
    admGlade glade;
    glade.init();
    if (!glade.loadFile("videoFilter/videoFilter.gtkBuilder"))
    {
        GUI_Error_HIG(QT_TR_NOOP("Cannot load dialog"), 
                      QT_TR_NOOP("File \"videoFilter.gtkBuilder\" could not be loaded."));
        return 0;
    }
    // create top window
    GtkWidget* dialog = glade.getWidget("dialogVFM");
    gtk_register_dialog(dialog);
   
    GtkWidget* treeFamilies = glade.getWidget("treeviewCategories");
    storeFamilies = glade.getWidget("liststoreCategories");
    treeAvailable = glade.getWidget("treeviewAvailable");
    GtkWidget* rendererAvailable = glade.getWidget("cellrendererAvailable");
    storeAvailable = glade.getWidget("liststoreAvailable");
    treeActive = glade.getWidget("treeviewActive");
    GtkWidget* rendererActive = glade.getWidget("cellrendererActive");
    storeActive = glade.getWidget("liststoreActive");
    
    g_signal_connect(G_OBJECT(treeAvailable), "size-allocate", G_CALLBACK(treeResize), (gpointer)rendererAvailable);
    g_signal_connect(G_OBJECT(treeActive), "size-allocate", G_CALLBACK(treeResize), (gpointer)rendererActive);

    // selecting a filter family will fill liststoreAvailable with its filters:
    g_signal_connect(G_OBJECT(treeFamilies), "cursor-changed", G_CALLBACK(displayFamily), NULL);

    // select the first category in treeFamilies, 
    // which will in turn fill liststoreAvailable:
    GtkTreePath* tp = gtk_tree_path_new_first();
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(treeFamilies), tp, NULL, TRUE);

    buildActiveFilterList();
    
    GtkWidget* buttonAdd = glade.getWidget("buttonAdd");
    GtkWidget* buttonConfigure = glade.getWidget("buttonConfigure");
    GtkWidget* buttonPreview = glade.getWidget("buttonPreview");
    GtkWidget* buttonSave = glade.getWidget("buttonSave");
    GtkWidget* buttonLoad = glade.getWidget("buttonLoad");
    GtkWidget* buttonUp = glade.getWidget("buttonUp");
    GtkWidget* buttonDown = glade.getWidget("buttonDown");
    GtkWidget* buttonRemove = glade.getWidget("buttonRemove");
    g_signal_connect(G_OBJECT(buttonAdd), "clicked", G_CALLBACK(addFilter), NULL);
    g_signal_connect(G_OBJECT(buttonConfigure), "clicked", G_CALLBACK(configureFilter), NULL);
    g_signal_connect(G_OBJECT(buttonPreview), "clicked", G_CALLBACK(previewFilter), NULL);
    g_signal_connect(G_OBJECT(buttonSave), "clicked", G_CALLBACK(saveFilters), NULL);
    g_signal_connect(G_OBJECT(buttonLoad), "clicked", G_CALLBACK(loadFilters), NULL);
    g_signal_connect(G_OBJECT(buttonUp), "clicked", G_CALLBACK(filterUp), NULL);
    g_signal_connect(G_OBJECT(buttonDown), "clicked", G_CALLBACK(filterDown), NULL);
    g_signal_connect(G_OBJECT(buttonRemove), "clicked", G_CALLBACK(filterRemove), NULL);
    g_signal_connect(G_OBJECT(treeAvailable), "row-activated", G_CALLBACK(availableDoubleClick), NULL);
    g_signal_connect(G_OBJECT(treeActive), "row-activated", G_CALLBACK(activeDoubleClick), NULL);
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_unregister_dialog(dialog);
    gtk_widget_destroy(dialog);
    return 1;
}

/**
    \fn    displayFamily
    \brief Show filters from the selected family in the available filters list
*/
void displayFamily(GtkTreeView *tree, gpointer data)
{
    uint32_t family;
    uint8_t ret = getSelectionNumber(VF_MAX, GTK_WIDGET(tree), GTK_LIST_STORE(storeFamilies), &family);
    gtk_list_store_clear(GTK_LIST_STORE(storeAvailable));
    uint32_t nb = ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family);
    ADM_info("Video filter Family: %u, nb %d\n", family, nb);
    GtkTreeIter iter;
    for (uint32_t i = 0; i < nb; i++)
    {
        const char *name, *desc;
        uint32_t major, minor, patch;
        ADM_vf_getFilterInfo((VF_CATEGORY)family, i, &name, &desc, &major, &minor, &patch);
        gchar* str = g_strconcat("<span weight=\"bold\">", name, "</span>\n",
                                 "<span size=\"smaller\">", desc, "</span>", NULL);
        guint tag = i + family*100;
        gtk_list_store_insert_with_values(GTK_LIST_STORE(storeAvailable), &iter, i, 0, str, 1, tag, -1);
        g_free(str);
    }
}

/**
    \fn    getSelectedRowData
    \brief Get data from the selected row and the specified column in a TreeView
*/
template<typename T>
bool getSelectedRowData(GtkWidget* tree, GtkWidget* store, guint column, T* data)
{
    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(selection, NULL, &iter) == FALSE)
    {
        ADM_warning("No selection\n");
        return false;
    }
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, column, data, -1);
    return true;
}

/**
    \fn    addFilter
    \brief Add selected filter to the active filters list
*/
void addFilter(GtkButton* button=0, gpointer data=0)
{
    VF_FILTERS tag;
    if (not getSelectedRowData(treeAvailable, storeAvailable, 1, &tag)) return;

    ADM_assert(tag < VF_MAX*100);
    int index = tag%100;
    int family = (tag-index)/100;
    ADM_info("Tag: %d->family=%d, index=%d\n", tag, family, index);
    ADM_assert(family < VF_MAX);
    ADM_assert(index < ADM_vf_getNbFiltersInCategory((VF_CATEGORY)family)); 
    
    ADM_vf_addFilterFromTag(video_body, tag, NULL, true);
    buildActiveFilterList();
}

void availableDoubleClick(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer)
{
    addFilter();
}

/**
    \fn    configureFilter
    \brief Configure the selected active filter
*/
void configureFilter(GtkButton* button=0, gpointer data=0)
{
    guint tag;
    if (not getSelectedRowData(treeActive, storeActive, 1, &tag)) return;

    ADM_info("Rank: %d\n", tag);
   
    ADM_vf_configureFilterAtIndex(tag);
    buildActiveFilterList();
}

void activeDoubleClick(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, gpointer)
{
    configureFilter();
}

/**
    \fn    previewFilter
    \brief Preview the selected active filter
*/
void previewFilter(GtkButton*, gpointer)
{
    // TODO
}

/**
    \fn    saveFilters
    \brief Save the active filter set to a file
*/
void saveFilters(GtkButton*, gpointer)
{
    // TODO
}

/**
    \fn    loadFilters
    \brief Load a filter set from a file
*/
void loadFilters(GtkButton*, gpointer)
{
    // TODO
}

/**
    \fn    filterUp
    \brief Move selected filter one place up
*/
void filterUp(GtkButton*, gpointer)
{
    guint tag;
    if (not getSelectedRowData(treeActive, storeActive, 1, &tag)) return;

    ADM_info("Rank: %d\n", tag);
    
    if (!tag) return;
    ADM_vf_moveFilterUp(tag);
    buildActiveFilterList();
    setSelectionNumber(nb_active_filter-1, treeActive, GTK_LIST_STORE(storeActive), tag-1);
}

/**
    \fn    filterDown
    \brief Move selected filter one place down
*/
void filterDown(GtkButton*, gpointer)
{
    guint tag;
    if (not getSelectedRowData(treeActive, storeActive, 1, &tag)) return;

    ADM_info("Rank: %d\n", tag);
     
    if (((int)tag < (int)(nb_active_filter-1)))
        {
            ADM_vf_moveFilterDown(tag);
            buildActiveFilterList();
            setSelectionNumber(nb_active_filter, treeActive, GTK_LIST_STORE(storeActive), tag+1);
        }
}

/**
    \fn    filterRemove
    \brief Remove selected filters from the active window list
*/
void filterRemove(GtkButton*, gpointer)
{
    guint tag;
    if (not getSelectedRowData(treeActive, storeActive, 1, &tag)) return;

    ADM_info("Deleting item %d\n", tag);

    ADM_vf_removeFilterAtIndex(tag);
    buildActiveFilterList();
    if (nb_active_filter)
    {
        if (tag > (nb_active_filter-1))
        {
            tag = nb_active_filter-1;
        }
        setSelectionNumber(nb_active_filter, treeActive, GTK_LIST_STORE(storeActive), tag);
    }
}

/**
    \fn    buildActiveFilterList
    \brief Build and display all active filters (may be empty)
*/
void buildActiveFilterList()
{
    gtk_list_store_clear(GTK_LIST_STORE(storeActive));
    int nb = ADM_vf_getSize();
    for (uint32_t i = 0; i < nb; i++)
    {
        uint32_t instanceTag = ADM_vf_getTag(i);
        ADM_coreVideoFilter* instance = ADM_vf_getInstance(i);
        const char* name = ADM_vf_getDisplayNameFromTag(instanceTag);
        const char* conf = instance->getConfiguration();
        printf("%d %s\n", i, name);
        gchar* str = g_strconcat("<span weight=\"bold\">", name, "</span>\n",
                                 "<span size=\"smaller\">", conf, "</span>", NULL);
        GtkTreeIter iter;
        gtk_list_store_insert_with_values(GTK_LIST_STORE(storeActive), &iter, i, 0, str, 1, i, -1);
        g_free(str);
    }
}

/**
    \fn    treeResize
    \brief Increase text width in available and active filter lists when the window is enlarged
*/
void treeResize(GtkWidget* tree, GtkAllocation* allocation, gpointer renderer)
{
    g_object_set(renderer, "wrap-width", allocation->width-8, NULL);
}
