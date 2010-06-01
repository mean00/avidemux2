#
# Generate glue to hook C functions to tinyPy static classes
# returnvalue FUNC param
#       param [int, float, void, str, couples]
#                        str = const char *
#                        couples =  confCouple *c
#
#  %funcss{funcName}
#  %proto{funcName}
#  %retType{funcName}
#  %params{funcname}
#
#


use strict;
my $input;
my $output;
my $headerFile;
my $line;
my $glueprefix="zzpy_";
my $functionPrefix="";
my $className;
my $cookieName;
my $cookieId;
my $staticClass=0;
#
my %cFuncs;
my %rType;
my %funcParams;
#
# Variables
my %getVar;
my %setVar;
my %typeVars;

sub debug
{
        my $str=shift;
        chomp($str);
        print OUTPUT "jsLog(\"$str\\n\");\n";

}
#
# processClass
#
sub processClass
{
        my $proto=shift;
        
        $proto=~s%.*\*/%%g;
        $proto=~s/ //g;
        #print "**$proto**\n";
        ($className,$cookieName,$cookieId)=split ":",$proto; 
        print "Processing class $className (with cookie=$cookieName,id=$cookieId)\n"; 
        if($cookieName=~m/void/)
        {
                $staticClass=1;
        }
        else
        {
                $staticClass=0;
        }

}
#
# Process a function declaration and write the glue code to do tinypy<->function call
#
sub processStaticFunc
{
        my $proto=shift;
        my $args=$proto;
        my @params;
        my $retType=$proto;;
        my $pyfunc=$proto;;
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
        $pyfunc=~s/ *\(.*$//g;
        $pyfunc=~s/^.* //g;
        ($cfunc,$pyfunc)=split ":",$pyfunc;
        $cFuncs{$pyfunc}=$cfunc;
        $rType{$pyfunc}=$retType;
        push @{$funcParams{$pyfunc}}, @params ; 
        #print " $pyfunc -> @params \n";
}

#
# Process a  STATICVAR :/* STATICVAR */ int    audioResample:scriptSetResample,scriptGetResample
#
sub processStaticVar
{
        my $proto=shift;
        my $args=$proto;
        my $type;
        my $varname;
        my @left;
        my @funcs;

        
        $args=~s/^.*\*\///g; # Remove comment

        @left=split(":",$args);
        @funcs=split(",",$left[1]);
        #print " >> ".$left[0]."<<\n";
        ($type,$varname)=split(" ",$left[0]);

        $type=~s/ //g;
        $varname=~s/ //g;

        #print "<$type> * <$varname> * <@funcs> *\n";
        $typeVars{$varname}=$type;
        $setVar{$varname}=$funcs[0];
        $getVar{$varname}=$funcs[1];
        print "Processing variable $varname of type $type\n";
}


#
# parsefile
#
sub parseFile
{
my $file=shift;
open(INPUT,$file) or die("Cannot open $file");
while($line=<INPUT>)
{
        chomp($line);
        if($line=~m/^#/)
        {
        }
        else
        {
            if($line=~m/\* STATICFUNC \*/)
            {
                my $proto=$line;
                # Remove header...
                # Remove tail
                $line=~s/^.*\*\///g;
                $line=~s/\/\/.*$//g;
                processStaticFunc($line);

            } elsif($line =~m/\/* CLASS \*/)
                {
                        processClass($line);
                }
                elsif($line =~m/\/* STATICVAR \*/)
                {
                        processStaticVar($line);
                }
}
}
close(INPUT);
}
#
#
#
sub genReturn
{
        my $retType=shift;
        if($retType=~m/int/)
        {
                return "  return tp_number(r);";
        }
        if($retType=~m/float/)
        {
                return "  return tp_number(r);";
        }
        if($retType=~m/void/)
        {
                return "";
        }
         if($retType=~m/str/)
        {
                return "  return tp_string(r);";
        }
return "???? $retType";
}
#
#
#
sub castFrom
{
        my $type=shift;
        if($type=~m/^str/)
        {
                return "pm.asString()";
        }
        if($type=~m/^int/)
        {
                return "pm.asDouble()";
        }
        if($type=~m/^float/)
        {
                return "pm.asFloat()";
        }
        if($type=~m/^double/)
        {
                return "pm.asDouble()";
        }
        print ">>>>>>>>>> <$type> unknown\n";
        return "????";

}
#
# Generate the locals / wrapping for the parameters
#
sub genParam
{
        my $num=shift;
        my $type=shift;
        my $r=1;
        $type=~s/ *$//g;
        $type=~s/^ *//g;
        if($type=~m/couples/)
        {
                print OUTPUT "  CONFcouple *p".$num."=NULL;\n";
                print OUTPUT "  pm.makeCouples(&p".$num.");\n";
                $r=0;
        }else
        {
                if($type=~m/str/)
                {
                        print OUTPUT "  const char *p".$num."=";
                }else
                {
                        print OUTPUT "  $type p".$num."=";
                }
                print OUTPUT " ".castFrom($type);
                print OUTPUT ";\n"; 
        } 
        return $r;
}
#
# Gen glue
#
sub genGlue
{
#
#  Generate function call glue
#
        my $i; 
        my $f;
        foreach $f(  keys %cFuncs)
        {
                my @params=@{$funcParams{$f}};
                my $pyFunc=$f;
                my $cfunc=$cFuncs{$f};
                my $ret=$rType{$f};
                my $nb=scalar(@params);
                
                print "Generating $pyFunc -> $ret $cfunc (@params ) \n";
                print OUTPUT "// $pyFunc -> $ret $cfunc (@params ) \n";
                # start our function
                print OUTPUT "static tp_obj ".$glueprefix.$f."(TP)\n {\n";

                debug("$f invoked\n");
                if($params[0]=~m/^void$/)
                {
                        $nb=0;
                }
                # unmarshall params...
                if($nb)
                {
                         print OUTPUT "  tinyParams pm(tp);\n";
                         #print OUTPUT "  void *me=pm.asThis(0);\n";
                         for($i=0;$i<$nb;$i++)
                         {
                                 genParam($i,$params[$i]);
                         }
                }
                # call function
                if(!($ret=~m/^void$/))
                {
                        if($ret=~m/str/)
                        {
                                print OUTPUT "  char *r=";
                        }else
                        {
                                print OUTPUT "  ".$ret." r=";
                        }
                }
                print OUTPUT "  ".$functionPrefix.$cfunc."(";
                for($i=0;$i<$nb;$i++)
                {
                        if($i)
                        {
                                print OUTPUT ",";
                        }
                        print OUTPUT "p".$i;
                } 
                print OUTPUT "); \n";
                # return value (if any)
                print OUTPUT genReturn($ret);
                print OUTPUT "\n}\n";

        }
#
# Generate get/set
#
        my $setName=$glueprefix."_".$className."_set";
        my $getName=$glueprefix."_".$className."_get";
# Get
        print OUTPUT "tp_obj ".$getName."(tp_vm *vm)\n";
        print OUTPUT "{\n";
        debug("$getName invoked\n");
        print OUTPUT "  tp_obj self=tp_getraw( vm);\n";
        print OUTPUT "  tinyParams pm(vm);\n";
        print OUTPUT "  void *me=pm.asThis(&self,$cookieId);\n";
        print OUTPUT "  char const *key = pm.asString();\n";
        my @k=keys %typeVars;
        my $v;
        foreach $v (@k)
        {
          print OUTPUT "  if (!strcmp(key, \"$v\"))\n";
          print OUTPUT "  {\n";
          print OUTPUT "     return tp_number(".$getVar{$v}."());\n";
          print OUTPUT "  }\n";
        }
        #print OUTPUT "  pm.raise(\"No such attribute %s\",key);\n";
        print OUTPUT "  return tp_get(vm,self,tp_string(key));\n";
        print OUTPUT "}\n";

# Set
        print OUTPUT "tp_obj ".$setName."(tp_vm *vm)\n";
        print OUTPUT "{\n";
        debug("$setName invoked\n");
        print OUTPUT "  tp_obj self=tp_getraw( vm);\n";
        print OUTPUT "  tinyParams pm(vm);\n";
        print OUTPUT "  void *me=pm.asThis(&self,$cookieId);\n";
        print OUTPUT "  char const *key = pm.asString();\n";
        my @k=keys %typeVars;
        my $v;
        foreach $v (@k)
        {
          print OUTPUT "  if (!strcmp(key, \"$v\"))\n";
          print OUTPUT "  {\n";
          print OUTPUT "     ".$typeVars{$v}." val=".castFrom($typeVars{$v}).";\n";
          print OUTPUT "     ".$setVar{$v}."(val);\n"; 
          print OUTPUT "     return tp_None;\n";
          print OUTPUT "  }\n";
        }
    #if (!strcmp(key, "prize")) return tp_number(fruit->prize);
        #print OUTPUT "  pm.raise(\"No such attribute %s\",key);\n";
        print OUTPUT "  return tp_None;\n";
        print OUTPUT "}\n";

}
#
#
#
sub genTables
{
   my $dtor=  "myDtor".$className;
# Dtor
        print OUTPUT "static void ".$dtor."(tp_vm *vm,tp_obj self)\n";
        print OUTPUT "{\n";
        debug("$dtor invoked\n");
        if($staticClass==1)
        {
        }else
        {
                #free(self.data.val);
        }
        print OUTPUT "}\n";
        # end
#
# Ctor
#
        # ctor
        print OUTPUT "static tp_obj myCtor".$className."(tp_vm *vm)\n";
        print OUTPUT "{\n";
        debug("ctor of $className invoked\n");
        print OUTPUT "  tp_obj self = tp_getraw(vm);\n";
        if($staticClass==1)
        {
                print OUTPUT "  void *me=NULL;\n";
        }else
        {

        }
        print OUTPUT "  tp_obj cdata = tp_data(vm, $cookieId, me);\n";
        print OUTPUT "  cdata.data.info->xfree = $dtor;\n";
        print OUTPUT "  tp_set(vm, self, tp_string(\"cdata\"), cdata);\n";
        print OUTPUT "  return tp_None;\n";
        print OUTPUT "}\n";
        # end
#
#  Generate help table
#
my $helpName=$glueprefix."_".$className."_help";
my $f;
my $cfunk;
my $pyFunc;
                print OUTPUT "static tp_obj ".$helpName."(TP)\n {\n";
                foreach $f(  keys %cFuncs)
                {
                        my @params=@{$funcParams{$f}};
                        print OUTPUT "  jsLog(\"$f(".join(",",@params) .")\");\n";
                }
                print OUTPUT "};\n";
#
#  Create the init function that will register our class
#
        my $setName=$glueprefix."_".$className."_set";
        my $getName=$glueprefix."_".$className."_get";

        print OUTPUT "tp_obj initClass".$className."(tp_vm *vm)\n";
        print OUTPUT "{\n";
        print OUTPUT "  tp_obj myClass=tp_class(vm);\n";
        print OUTPUT "  tp_set(vm,myClass,tp_string(\"__init__\"),tp_fnc(vm,myCtor".$className."));\n";
        print OUTPUT "  tp_set(vm,myClass,tp_string(\"__set__\"),tp_fnc(vm,".$setName."));\n";
        print OUTPUT "  tp_set(vm,myClass,tp_string(\"__get__\"),tp_fnc(vm,".$getName."));\n";
        print OUTPUT "  tp_set(vm,myClass,tp_string(\"help\"),tp_fnc(vm,$helpName));\n";
        #
        foreach $f(  keys %cFuncs)
        {
                my $pyFunc=$f;
                $cfunk=$glueprefix.$f;

                print OUTPUT "  tp_set(vm,myClass,tp_string(\"$pyFunc\"),tp_fnc(vm,$cfunk));\n";
        }
        print OUTPUT "  return myClass;\n";
        print OUTPUT "}\n";
}
##################################
#  Main
##################################
if(scalar(@ARGV)!=1)
{
        die("admPy inputfile\n");
}
$input=$ARGV[0];
$output=$input;
$output=~s/\..*$//g;
my $thisfile=$output;
$thisfile=~s/^.*\///g;
$headerFile=$output."_gen.h";
$output=$output."_gen.cpp";
##
## Main Loop
##
# 1 grab all functions
parseFile($input);
# 2 gen glue
open(OUTPUT,">$output") or die("Cannot open $output");
print OUTPUT "// Generated by admPyClass.pl do not edit !\n";
genGlue();
#
# 3 gen tables (ctor, help, register)
#
genTables();
close(OUTPUT);

      
#
print "done\n.";
