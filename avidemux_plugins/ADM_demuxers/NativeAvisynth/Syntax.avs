# Generates colorbar image and converts it to YUY2
# The image and sound is now "current clip"

Colorbars(320,240)
ConvertToYUY2()

# Take the "current clip", and add greayscale to this.
# The result is now in the variable Grey.
# Current clip is still the original Colorbars(320,240)

Grey = Greyscale()

# Now you can use Grey as an alternative clip, instead if the current clip
# This creates a new variable called Grey_Half_Size, that contains the grey colorbar at half the height.
# Current clip is still the original Colorbars(320,240)

Grey_Half_Size = VerticalReduceBy2(Grey)

# If we specify more parameters, but don't want it used on the current clip, 
#  we can either specify the clip in front of the filter, using '.' 
#  or specify the clip as the first parameter.

Grey_Same_Size = Grey_half_size.AddBorders(0,60,0,60)

# which is the same as:

Grey_Same_Size = AddBorders(Grey_half_size,0,60,0,60)

# More examples:

Combined = MergeLuma(grey_same_size)
#Combined = MergeChroma(Combined, grey_same_size)
#Grey_same_size=mergeluma(combined)
# Add titles to clips:

Thisclip = Subtitle("Current Clip")
Grey_Half_size = Subtitle(Grey_Half_size,"Grey_Half_size")
Grey_Same_size = Subtitle(Grey_Same_size,"Grey_Same_size")
Combined = Subtitle(Combined,"Combined")

# Put them on top of eachother

Stackvertical(thisclip, Grey_Half_size, Grey_Same_size, Combined)

ConvertToYV12(thisclip)