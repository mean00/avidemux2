use strict;
my $input;
my $output;
my $line;
my $glueprefix="zzpy_";
my $functionPrefix="";
my @allFuncs;
#
#
#
sub genReturn
{
        my $retType=shift;
        if($retType=~m/int/)
        {
                return "return tp_number(r);";
        }
        if($retType=~m/float/)
        {
                return "return tp_number(r);";
        }
        if($retType=~m/void/)
        {
                return "return tp_None;";
        }
        return "???? $retType";
}
#
#
#
sub castFrom
{
        my $type=shift;
        if($type=~m/^char/)
        {
                return "(char *)TP_STR().string.val";
        }
        if($type=~m/^int/)
        {
                return "TP_NUM()";
        }
        if($type=~m/^float/)
        {
                return "TP_NUM()";
        }
        return "????";

}
#
# Generate the locals / wrapping for the parameters
#
sub genParam
{
        my $num=shift;
        my $type=shift;
        print OUTPUT "$type p".$num."=";
        print OUTPUT castFrom($type);
        print OUTPUT ";\n"; 
}
#
# Process a function declaration and write the glue code to do tinypy<->function call
#
sub processFunc
{
        my $proto=shift;
        my $args=$proto;
        my @params;
        my $retType=$proto;;
        my $func=$proto;;
        my $cfunc;
        $args=~s/^.*\(//g;
        $args=~s/\).*$//g;
        #print "args => $args\n";
        @params=split ",",$args;
        #print "params -> @params\n";
        # Get return type...
        $retType=~s/^ *//g;
        $retType=~s/ .*$//g;
        # get functioName
        $func=~s/ *\(.*$//g;
        $func=~s/^.* //g;
        ($cfunc,$func)=split ":",$func;
        print OUTPUT "//$retType  $cfunc <@params>\n";
        # Write glue code
        print OUTPUT "tp_obj ".$glueprefix.$func."(TP)\n";
        push(@allFuncs,$func);
        print OUTPUT "{\n";
        my $nb=scalar(@params);
        my $i;
        for($i=0;$i<$nb;$i++)
        {
                $params[$i]=~s/^ *//g; # Remove " " at the beginning if any
                $params[$i]=~s/ *$//g; # Remove " " at the end if any
        }
        if($params[0]=~m/^void$/)
        {
                $nb=0;
        }
        print " New function : $retType,$func,<@params>\n";
        


        for($i=0;$i<$nb;$i++)
        {
                genParam($i,$params[$i]);
        }
        # Call real function
        if(!($retType=~m/^void$/))
        {
                print OUTPUT $retType." r=";
        }
        print OUTPUT $functionPrefix.$cfunc."(";
        for($i=0;$i<$nb;$i++)
        {
                if($i)
                {
                        print OUTPUT ",";
                }
                print OUTPUT "p".$i;
        } 
        print OUTPUT "); \n";
        # Cast r to pyobj
        print OUTPUT genReturn($retType);
        print OUTPUT "\n}\n";


}


if(scalar(@ARGV)!=2)
{
        die("admPy inputfile outputfile\n");
}
$input=$ARGV[0];
$output=$ARGV[1];
print "Processing $input=>$output\n";
open(OUTPUT,">$output") or die("Cannot open $output");
##
## Main Loop
##
# 1 grab all functions
open(INPUT,$input) or die("Cannot open $input");
close(INPUT);
# Process them
open(INPUT,$input) or die("Cannot open $input");
while($line=<INPUT>)
{
        chomp($line);
        if($line=~m/^#/)
        {
        }
        else
        {
            if($line=~m/\* FUNC \*/)
            {
                my $proto=$line;
                # Remove header...
                # Remove tail
                $line=~s/^.*\*\///g;
                $line=~s/\/\/.*$//g;
                processFunc($line);

            }
        }
}
# gen init array
my $nbFunc=scalar(@allFuncs);
my $i;
my $thisfile=$input;
$thisfile=~s/\.[a-zA-Z]*$//g;
$thisfile=~s/^.*\///g;
print "creating  $thisfile function table\n";
print OUTPUT "pyFuncs ".$thisfile."_functions[]={\n";
for($i=0;$i<$nbFunc;$i++)
{
                my $line=$allFuncs[$i];
                #print "Function : $line\n";
                my $funcName;
                my $funcProto;
                $funcName="\"".$line."\"";
                $funcProto=$glueprefix.$line;
                print OUTPUT "{$funcName,$funcProto},\n";
}
print OUTPUT "{NULL,NULL}\n};\n";
close(INPUT);
close(OUTPUT);
print "done\n.";
