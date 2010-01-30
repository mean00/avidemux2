#!/usr/bin/perl -w

my %ctypes;
my %atypes;
my $input;
my $structName;
#
#
#
$ctypes{"uint32_t"}="uint32_t";
$ctypes{"int32_t"}="int32_t";
$ctypes{"bool"}="bool";
$ctypes{"string"}="char *";
$ctypes{"bool"}="bool";
$ctypes{"float"}="float";
$ctypes{"video_encode"}="COMPRES_PARAMS";
$ctypes{"lavcodec_context"}="FFcodecContext";

$atypes{"uint32_t"}="ADM_param_uint32_t";
$atypes{"int32_t"}="ADM_param_int32_t";
$atypes{"float"}="ADM_param_float";
$atypes{"bool"}="ADM_param_bool";
$atypes{"string"}="ADM_param_string";
$atypes{"float"}="ADM_param_float";
$atypes{"video_encode"}="ADM_param_video_encode";
$atypes{"lavcodec_context"}="ADM_param_lavcodec_context";

#
sub processLine
{

        my $type=shift;
        my $varName=shift;
        my $ctype;
        my $atype;

        #print "<$type,$varName>\n";

        $ctype=$ctypes{$type};
        if(!defined($ctype)){ die("Type <$type> unknown\n");}
        $atype=$atypes{$type};
        if(!defined($atype)){ die("aType <$type> unknown\n");}

        #print "$type=>$ctype\n";
        print OUT_H "   $ctype $varName;\n";

        print OUT_DESC " {\"$varName\",offsetof( $structName,$varName),\"$ctype\",$atype},\n";

}
sub writeDescHead
{
        print OUT_DESC "// Automatically generated, do not edit!\n";
        print OUT_DESC "const ADM_paramList ".$structName."_param[]={\n";


}
sub writeDescFoot
{
        print OUT_DESC "{NULL,0,NULL}\n";
        print OUT_DESC "};\n";


}
sub writeHeaderHead
{
        print OUT_H "// Automatically generated, do not edit!\n";
        print OUT_H "#ifndef ADM_".$structName."_CONF_H\n";
        print OUT_H "#define ADM_".$structName."_CONF_H\n";
        print OUT_H "typedef struct {\n";

}
sub writeHeaderFoot
{
        my $str=shift;
        print OUT_H "}$structName;\n";
        print OUT_H "#endif //".$structName."\n";
        print OUT_H "//EOF\n";

}
#
# Main
#
if(scalar(@ARGV)==0)
{
        die("Need at least one arg!");
}
$input=$ARGV[0];
$structName=$input;
$structName=~s/\.conf//g;
$output_h=$structName.".h";
$output_desc=$structName."_desc.cpp";

open(IN,$input) || die("Cannot open $input for reading");
open(OUT_H,">$output_h") || die("Cannot open $output_h to write header");
open(OUT_DESC,">$output_desc") || die ("cannot open $output_desc to write struc");
writeHeaderHead();
writeDescHead();

while($a=<IN>)
{
        chomp($a);
        $a=~s/#.*$//g;
        $a=~s/^[ \t]*//g;
        my $type;
        my $name;
        ($type,$name)=split ":",$a;
        #print "1: ".$a.",2:".$type.",3:".$name."\n" ;
        $name=~s/;.*$//g;
        processLine($type,$name);
}
writeHeaderFoot();
writeDescFoot();
close(IN);
close(OUT_H);
close(OUT_DESC);
print "Done.\n" ;

