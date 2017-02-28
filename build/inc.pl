#/*<

no warnings 'redefine';
no warnings 'experimental';
use strict;
#use feature 'signatures';

use Data::Dumper;

=pod
our %c;
sub collect {
    my ($a, $b) = @_;
    local $Data::Dumper::Purity = 1;
    local $Data::Dumper::Indent = 0;
    local $Data::Dumper::Sortkeys = 1;
    $c{$a}{Data::Dumper->Dump([$b], [qw($_)])} = 1;
    #say Data::Dumper->Dump([$b], [qw($_)]);
}

sub dispatch {
    my ($a) = @_;
    if (exists($c{$a})) {
        return map { eval $_; } sort keys %{$c{$a}};
    } else {
        return ();
    }
    
}
=cut

our %c2;
# collect('collection1', 'name1', {...});
# or
# collect('collection1', {name=>'name1',...});
sub collect {
    my $collection = shift;
    my $name = shift; # object or name
    my $obj;
    if (ref($name) eq 'HASH') {
        local $Data::Dumper::Indent = 0;
        local $Data::Dumper::Sortkeys = 1;
        $obj = $name;
        $obj->{name} = $obj->{name} // Dumper($obj);
        $name = $obj->{name};

        #delete $obj->{name};
    } else {
        $obj = shift // {};
        $obj->{name} = $name;
    }
    # union
    if (exists($c2{$collection}{$name})) {
        $obj = \(%{$c2{$collection}{$name}}, %{$obj});
    }
    $c2{$collection}{$name} = $obj;
}

sub dispatch {
    my $collection = shift;
    if (exists($c2{$collection})) {
        return sort { $a->{name} cmp $b->{name} } values %{$c2{$collection}};
    } else {
        return ();
    }
}

sub dispatch_s {
    my $collection = shift;
    if (exists($c2{$collection})) {
        return sort keys %{$c2{$collection}};
    } else {
        return ();
    }
}

=pod
use Set::Scalar;

our %c;
sub collect {
    my $k = shift;
    $c{$k} = Set::Scalar->new unless exists($c{$k});
    $c{$k}->insert(@_);
}
sub dispatch {
    my $k = shift;
    if (exists($c{$k})) {
        return sort $c{$k}->members;
    } else {
        return ();
    }
}

=cut


# fff(_XXX("gggh\")ff")

sub register_component {
    collect('component_headers',  $filename);
    collect('components', {@_});
}

sub register_module {
    collect('module_headers',  $filename);
    collect('modules', {@_});
}


sub get_components {
   sort { $a->{priority} <=> $b->{priority} } dispatch('components'); 
}


sub register_callback {
    collect('callbacks', {type=>shift, name=>shift});
}




#print map { '#include "' . $_->{'file'} . "\"\n" } dispatch('component') ;
# for my $c (dispatch('component')) {
    
#     #print $c->{'class'}, "\n";
# }

#register_component('abc_t', 'abc');
#register_component('def_t', 'def');

#print  dispatch('yyyy');
#print join "\n", (dispatch('yyyy'));

# fff(_XXX("gggh")ff") fff(_XXX("gggh")ff")

# collect('yyyy', {'abc' => 3, 'cde' => 6});
# collect('yyyy', [1,2,3,4]);
# collect('yyyy', {'abc' => 3, 'cde' => 6});
# print Dumper(dispatch('yyyy'));


# $filehandlers{'test handler'} = sub {
#     my ($fn, $content) = @_;
#     while ($$content =~ m{\W_XXX\("((?:[^"\\]|\\.)+)"\)}gs) {
#         collect('yyyy', $1);
#     }
# }


=pod
sub find_brace {
    my ($str, $pos, $open, $close) = @_;

    my $ch;
    my  $balance = 1;
    my $in_str = 0;
    while ($balance != 0 && $pos < length($$str)) {
        $ch = substr $$str, ++$pos, 1;
        if($ch eq q(") || $ch eq q(')) {
            $in_str = !$in_str;
            next;
        }
        if ($in_str)
        {
            if ($ch eq '\\')
            {
                $pos++;
                next;
            }
        }
        else {
            if ($ch eq $open)  {
                $balance++;
            }
            elsif ($ch eq $close) {
                $balance--;
            }
        }
    }

    return $pos;
}


sub cpp_fun {
    $_ = shift;
    my %fun;
    #s%/\*.*$%%s;
    if (m/^
        \s*
        ([\w\s]*)? # static etc modifiers
        \s+
        ([\w:&*]+) # type
        \s+
        ([\w:]+) # function name
        
        ([\w\s]*)? # const etc modifiers

        \((
        .*? # parameters
        )\)
        \s*
        {
        /asx) {

        my $end = find_brace(\$_, $+[0], '{', '}');
        $fun{'body'} = substr($_, $+[0], $end - $+[0]);

        $fun{'params'} = [];

        $fun{'1'} = $1;
        $fun{'ret'} = $2;
        $fun{'id'} = $3;
        $fun{'2'} = $4;

        #print $1, "\n2: $2\n 3 : $3\n 4 : $4\n 5: $5";
        
        for (split /,/, $5) {
            #print "$_\n";
            if(m/
               ^
               \s*
               ([\w\s:*&]+) # type
               \s+
               (\w+) # varname
               (\s*=.*)?
               $
               /asx) {
                my $var = {
                    'type' => $1,
                    'name' => $2,
                };

                $$var{'def'} = $3 if defined($3);

                push @{$fun{'params'}}, $var;
                #print "var type: $1 name: $2 def: $3\n";
            }
        }
    } 
    else {
        die('parse error!');
    }
    use Data::Dumper;
    Dumper(\%fun);
}
=cut

# $filehandlers{'glfuncs'} = sub {
#     my ($fn, $content) = @_;
#     while ($$content =~ m{gl::(\w+)}ga) {
#         collect('glfuncs', $1);
#     }
# }


sub serialize {
    'template <class Archive> void serialize(Archive & ar, const unsigned int) { ar & ' . ( join ' & ', 
#map { "BOOST_SERIALIZATION_NVP($_)" } 
@_ ) . '; }';
}

sub serialize_free_nvp {
'
namespace boost { namespace serialization {
template <class Archive> void serialize(Archive & ar, ' . shift . ' & t, const unsigned int) {
ar & ' . ( join ' & ', map { qq[make_nvp("$_", t.$_)] } @_ ) . ';
}}}
';
}

sub serialize_free {
    'template <class Archive> void serialize(Archive & ar, ' . shift . ' & t, const unsigned int) { ar & ' . ( join ' & ', @_ ) . '; }';
}

#>*/
