TGT=../avidemux_icon_small.inc \
../gnome-calculator.inc \
../gnome-calculator_small.inc \
../stock_allow-effects.inc \
../1.inc \
../2.inc \
../3.inc \
../4.inc \
../5.inc \
../6.inc \
../7.inc \
../preview.inc \
../output.inc \
../stock_filters-aging.inc \
../systray.inc \
../systray2.inc \
../film1.inc \
../film3.inc \
../film5.inc \
../film7.inc \
../film9.inc \
../film11.inc \
../film13.inc \
../film15.inc \
../film17.inc \
../film19.inc \
../film21.inc \
../film23.inc \
../audio-volume-medium.inc \
../preview-button.inc

../%.inc : %.png	
	echo $<
	gdk-pixbuf-csource --raw --name=`basename $@ .inc` $< > $@
all : $(TGT)

clean:
	rm -f $(TGT) 
