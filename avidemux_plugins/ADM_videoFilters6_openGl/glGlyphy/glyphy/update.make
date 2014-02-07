SHADERS = \
        glyphy-common.glsl \
        glyphy-sdf.glsl \
        $(NULL)
SHADERHEADERS = $(patsubst %.glsl,%-glsl.h, $(SHADERS))


%-glsl.h: %.glsl stringize
	$(AM_V_GEN) ./stringize "static const char *`echo "$<" | \
	sed 's@.*/@@;s/[-.]/_/g'`" < "$<" > "$@.tmp" && \
	mv "$@.tmp" "$@" || ($(RM) "$@.tmp"; false)


all: $(SHADERHEADERS)
	
