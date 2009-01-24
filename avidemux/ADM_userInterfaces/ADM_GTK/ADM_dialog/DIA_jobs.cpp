#include "ADM_toolkitGtk.h"
#include "DIA_coreToolkit.h"

static GtkWidget       *create_dialog1 (void);
static void             updateStatus(void);
static int              getSelection(GtkWidget *dialog);
extern bool parseECMAScript(const char *name);
static const char *StringStatus[]={QT_TR_NOOP("Ready"),QT_TR_NOOP("Succeeded"),QT_TR_NOOP("Failed"),QT_TR_NOOP("Deleted"),QT_TR_NOOP("Running")};


typedef enum
{
        STATUS_READY=0,
        STATUS_SUCCEED,
        STATUS_FAILED,
        STATUS_DELETED,
        STATUS_RUNNING
}JOB_STATUS;


typedef struct
{
  JOB_STATUS  status;
  ADM_date    startDate;
  ADM_date    endDate;
}ADM_Job_Descriptor;

typedef enum
{
        COMMAND_DELETE_ALL=1,
        COMMAND_DELETE=2,
        COMMAND_RUN_ALL=3,
        COMMAND_RUN=4
};


typedef struct
{
        GtkWidget *dialog;
        GtkListStore *store;
        uint32_t  nb;
        char      **name;
        ADM_Job_Descriptor *status;
}JobsDescriptor;

static JobsDescriptor jobs;

uint8_t  DIA_job(uint32_t nb, char **name)
{
GtkListStore *store;

GtkTreeViewColumn *column,*column2,*column3;
GtkCellRenderer *renderer;

        int ret=0;



        GtkWidget *dialog;

        dialog=create_dialog1();

		gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog),
									GTK_RESPONSE_OK,
									GTK_RESPONSE_CANCEL,
									-1);

        gtk_register_dialog(dialog);


        store=gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING,G_TYPE_STRING);

        // initialize our job structure
        jobs.dialog=dialog;
        jobs.nb=nb;
        jobs.name=name;
        jobs.status=new ADM_Job_Descriptor[nb];
        jobs.store=store;
        memset(jobs.status,0,jobs.nb*sizeof(ADM_Job_Descriptor));

        gtk_tree_view_set_model(GTK_TREE_VIEW(WID(treeview1)),GTK_TREE_MODEL (store));
        gtk_tree_view_columns_autosize(GTK_TREE_VIEW(WID(treeview1)));

        // Add columns

        renderer = gtk_cell_renderer_text_new ();
        column = gtk_tree_view_column_new_with_attributes (QT_TR_NOOP("  Job Name  "), renderer,
                                                      "markup", (GdkModifierType) 0,
                                                      NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (WID(treeview1)), column);

        column2 = gtk_tree_view_column_new_with_attributes (QT_TR_NOOP("Started at"), renderer,
                                                      "markup", (GdkModifierType) 1,
                                                      NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (WID(treeview1)), column2);
        column3 = gtk_tree_view_column_new_with_attributes (QT_TR_NOOP("Finished at"), renderer,
            "markup", (GdkModifierType) 2,
            NULL);
        gtk_tree_view_append_column (GTK_TREE_VIEW (WID(treeview1)), column3);

        //
        #define ASSOCIATE(x,y)   gtk_dialog_add_action_widget (GTK_DIALOG (dialog), WID(x),y)
            ASSOCIATE(buttonDelete,COMMAND_DELETE);
            ASSOCIATE(buttonDeleteAll,COMMAND_DELETE_ALL);
            ASSOCIATE(buttonRunAll,COMMAND_RUN_ALL);
            ASSOCIATE(buttonRun,COMMAND_RUN);
        //

        int running=1;
        gtk_widget_set_usize(WID(treeview1), 180, 300);
        while(running)
        {
                int sel=0,event;
                updateStatus();
                switch(event=gtk_dialog_run(GTK_DIALOG(dialog)))
                {
                        case GTK_RESPONSE_OK : running=0;break;
                        case GTK_RESPONSE_APPLY : running=0;break;
                        case GTK_RESPONSE_CANCEL :
                        case GTK_RESPONSE_DELETE_EVENT:
                                         running=0;break;
                        case COMMAND_DELETE_ALL:
                                        if(GUI_Confirmation_HIG(QT_TR_NOOP("Sure!"),QT_TR_NOOP("Delete ALL jobs"),QT_TR_NOOP("Are you sure you want to delete all jobs ?")))
                                        {
                                                for(int i=0;i<jobs.nb;i++) jobs.status[i].status=STATUS_DELETED;
                                        }
                                        break;
                        case COMMAND_RUN:
                                        sel=getSelection(jobs.dialog);
                                        if(sel>=jobs.nb) break;
                                        jobs.status[sel].status=STATUS_RUNNING;
                                        updateStatus();
                                        GUI_Quiet();
                                        TLK_getDate(&(jobs.status[sel].startDate));
                                        if(parseECMAScript(jobs.name[sel])) jobs.status[sel].status=STATUS_SUCCEED;
                                        else jobs.status[sel].status=STATUS_FAILED;
                                        TLK_getDate(&(jobs.status[sel].endDate));
                                        updateStatus();
                                        GUI_Verbose();
                                        break;
                        case COMMAND_RUN_ALL:
                                        GUI_Quiet();
                                        for(int i=0;i<jobs.nb;i++)
                                        {
                                          if(jobs.status[i].status==STATUS_DELETED) continue;
                                          if(jobs.status[i].status==STATUS_SUCCEED) continue;
                                          jobs.status[i].status=STATUS_RUNNING;
                                          TLK_getDate(&(jobs.status[i].startDate));
                                          updateStatus();
                                          if(parseECMAScript(jobs.name[i])) jobs.status[i].status=STATUS_SUCCEED;
                                                        else jobs.status[i].status=STATUS_FAILED;
                                        TLK_getDate(&(jobs.status[i].endDate));

                                        }
                                        updateStatus();
                                        GUI_Verbose();
                                        break;
                        case COMMAND_DELETE:
                                        sel=getSelection(jobs.dialog);
                                        if(sel>=jobs.nb) break;
                                        if(GUI_Confirmation_HIG(QT_TR_NOOP("Sure!"),QT_TR_NOOP("Delete job"),QT_TR_NOOP("Are you sure you want to delete %s job ?"),ADM_GetFileName(jobs.name[sel])))
                                        {
                                                jobs.status[sel].status=STATUS_DELETED;
                                        }
                                        break;


                        default:
                                printf("Event:%d\n",event);
                                GUI_Error_HIG("Jobs",QT_TR_NOOP("Unknown event"));break;
                }

        }
        gtk_unregister_dialog(dialog);
        gtk_widget_destroy(dialog);

        // Now delete the "deleted" jobs
        for(int i=0;i<jobs.nb;i++)
        {
                if(jobs.status[i].status==STATUS_DELETED)
                {
                        unlink(jobs.name[i]);
                }

        }

        delete [] jobs.status;

        return ret;
}
//**************************************************
int              getSelection(GtkWidget *dialog)
{
uint32_t n=0xffff;
        if(! getSelectionNumber(jobs.nb,WID(treeview1) , jobs.store,&n)) return 0xffff;
        return n;
}
//*************************************
void updateStatus(void)
{
GtkTreeIter iter;
char *str;
ADM_date  *date;
char *str1,str2[200],str3[200];

        gtk_list_store_clear (jobs.store);
        for (uint32_t i = 0; i < jobs.nb; i++)
        {
               str1 = g_markup_printf_escaped("<span weight=\"heavy\">%s</span>\n"
               "<span size=\"smaller\" style=\"oblique\">%s</span>"
                , ADM_GetFileName(jobs.name[i]), StringStatus[jobs.status[i].status]);

                date=&(jobs.status[i].startDate);
                sprintf(str2,"%02d:%02d:%02d",date->hours,
                                          date->minutes,
                                          date->seconds);

                date=&(jobs.status[i].endDate);
                sprintf(str3,"%02d:%02d:%02d",date->hours,
                        date->minutes,
                        date->seconds);

                gtk_list_store_append (jobs.store, &iter);
                gtk_list_store_set (jobs.store, &iter, 0,str1,1,str2,2,str3,-1);
                printf("Start : %s\n",str2);
                printf("End : %s\n",str3);

				g_free(str1);
        }
}
//*************************************

GtkWidget*
create_dialog1 (void)
{
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *hbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *treeview1;
  GtkWidget *vbuttonbox1;
  GtkWidget *buttonDeleteAll;
  GtkWidget *buttonDelete;
  GtkWidget *buttonRunAll;
  GtkWidget *buttonRun;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), QT_TR_NOOP("Jobs"));
  gtk_window_set_type_hint (GTK_WINDOW (dialog1), GDK_WINDOW_TYPE_HINT_DIALOG);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

  treeview1 = gtk_tree_view_new ();
  gtk_widget_show (treeview1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), treeview1);
  //gtk_widget_set_size_request (treeview1, 300, -1);

  vbuttonbox1 = gtk_vbutton_box_new ();
  gtk_widget_show (vbuttonbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbuttonbox1, FALSE, FALSE, 0);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (vbuttonbox1), GTK_BUTTONBOX_START);

  buttonDeleteAll = gtk_button_new_with_mnemonic (QT_TR_NOOP("Delete All Jobs"));
  gtk_widget_show (buttonDeleteAll);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), buttonDeleteAll);
  GTK_WIDGET_SET_FLAGS (buttonDeleteAll, GTK_CAN_DEFAULT);

  buttonDelete = gtk_button_new_with_mnemonic (QT_TR_NOOP("Delete Job"));
  gtk_widget_show (buttonDelete);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), buttonDelete);
  GTK_WIDGET_SET_FLAGS (buttonDelete, GTK_CAN_DEFAULT);

  buttonRunAll = gtk_button_new_with_mnemonic (QT_TR_NOOP("Run all jobs"));
  gtk_widget_show (buttonRunAll);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), buttonRunAll);
  GTK_WIDGET_SET_FLAGS (buttonRunAll, GTK_CAN_DEFAULT);

  buttonRun = gtk_button_new_with_mnemonic (QT_TR_NOOP("Run Job"));
  gtk_widget_show (buttonRun);
  gtk_container_add (GTK_CONTAINER (vbuttonbox1), buttonRun);
  GTK_WIDGET_SET_FLAGS (buttonRun, GTK_CAN_DEFAULT);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), cancelbutton1, GTK_RESPONSE_CANCEL);
  GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (dialog1), okbutton1, GTK_RESPONSE_OK);
  GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog1, "dialog1");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (dialog1, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (dialog1, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (dialog1, treeview1, "treeview1");
  GLADE_HOOKUP_OBJECT (dialog1, vbuttonbox1, "vbuttonbox1");
  GLADE_HOOKUP_OBJECT (dialog1, buttonDeleteAll, "buttonDeleteAll");
  GLADE_HOOKUP_OBJECT (dialog1, buttonDelete, "buttonDelete");
  GLADE_HOOKUP_OBJECT (dialog1, buttonRunAll, "buttonRunAll");
  GLADE_HOOKUP_OBJECT (dialog1, buttonRun, "buttonRun");
  GLADE_HOOKUP_OBJECT_NO_REF (dialog1, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (dialog1, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (dialog1, okbutton1, "okbutton1");

  return dialog1;
}

