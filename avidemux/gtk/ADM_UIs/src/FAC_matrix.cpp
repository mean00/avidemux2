/***************************************************************************
  FAC_matrix.cpp
  Handle dialog factory element : Matrix
  (C) 2007 Mean Fixounet@free.fr 
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ADM_toolkitGtk.h"
#include "DIA_factory.h"
namespace ADM_GtkFactory
{
class diaElemMatrix : public diaElem
{
  protected:
  public:
    uint8_t *_matrix;
    uint32_t _matrixSize;
    		diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip=NULL);
  virtual   ~diaElemMatrix() ;
  void      setMe(void *dialog, void *opaque,uint32_t line);
  void      getMe(void);
  void      enable(uint32_t onoff) ;
  int getRequiredLayout(void);
};

diaElemMatrix::diaElemMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip)
  : diaElem(ELEM_MATRIX)
{
  param=(void *)trix;
  paramTitle=toggleTitle;
  this->tip=tip;
  _matrix=new uint8_t[trixSize*trixSize];
  _matrixSize=trixSize;
  memcpy(_matrix,trix,trixSize*trixSize);
}

diaElemMatrix::~diaElemMatrix()
{
  if(_matrix) delete [] _matrix;
  _matrix=NULL;
  if(myWidget)
  {
	  GtkWidget **arrayWidget=( GtkWidget **)myWidget;
	  delete [] arrayWidget;
	  myWidget=NULL;
  }
}
void diaElemMatrix::setMe(void *dialog, void *opaque,uint32_t line)
{
  GtkWidget *widget;
  GtkWidget *table;
  GtkWidget *label;
  GtkWidget **arrayWidget=new GtkWidget*[_matrixSize*_matrixSize];  
  label = gtk_label_new_with_mnemonic (paramTitle);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_widget_show(label);
  
  gtk_table_attach (GTK_TABLE (opaque), label, 0, 1, line, line+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  //*************
    table = gtk_table_new (_matrixSize, _matrixSize, FALSE);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 0);
    gtk_widget_show(table);
    
    gtk_table_attach (GTK_TABLE (opaque), table, 1, 2, line, line+1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  
  // Create widgets
    int index=0;
    for(int y=0;y<_matrixSize*_matrixSize;y++)
    	{
    		int val=_matrix[index];
    		GtkWidget *w= gtk_spin_button_new_with_range(0,255,1);
    		
    		  gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(w),TRUE);
    		  gtk_spin_button_set_digits  (GTK_SPIN_BUTTON(w),0);
    		  gtk_spin_button_set_value (GTK_SPIN_BUTTON(w),val);
    		  arrayWidget[index]=w;
    		  
    		  gtk_table_attach (GTK_TABLE (table),w, y%_matrixSize, 1+(y%_matrixSize), y/_matrixSize, 1+(y/_matrixSize),
    		                     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
    		                     (GtkAttachOptions) (0), 0, 0);
    		  gtk_widget_show(w);
    		  index++;
    	}
    	
    
  
    myWidget=(void *)arrayWidget;
    if(tip)
    {
      GtkTooltips *tooltips= gtk_tooltips_new ();
      gtk_tooltips_set_tip (tooltips, table, tip, NULL);
    }

}
void diaElemMatrix::getMe(void)
{
	 GtkWidget **arrayWidget=( GtkWidget **)myWidget;
	 
	 ADM_assert(arrayWidget);
	 int index=0;
	  for(int y=0;y<_matrixSize*_matrixSize;y++)
	  {
	    		_matrix[index]=(uint8_t)gtk_spin_button_get_value( GTK_SPIN_BUTTON(arrayWidget[index]));
	    		index++;
	  }
	  memcpy(param,_matrix,_matrixSize*_matrixSize);
}
void diaElemMatrix::enable(uint32_t onoff)
{
	 GtkWidget **arrayWidget=( GtkWidget **)myWidget;
		 
		 ADM_assert(arrayWidget);
		 int index=0;
		  for(int y=0;y<_matrixSize*_matrixSize;y++)
		  {
			  gtk_widget_set_sensitive( GTK_WIDGET(arrayWidget[index++]),onoff);
		  }
}

int diaElemMatrix::getRequiredLayout(void) { return 0; }
} // End of namespace
//****************************Hoook*****************

diaElem  *gtkCreateMatrix(uint8_t *trix,const char *toggleTitle, uint32_t trixSize,const char *tip)
{
	return new  ADM_GtkFactory::diaElemMatrix(trix,toggleTitle,trixSize,tip);
}
void gtkDestroyMatrix(diaElem *e)
{
	ADM_GtkFactory::diaElemMatrix *a=(ADM_GtkFactory::diaElemMatrix *)e;
	delete a;
}
//EOF
