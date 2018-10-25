use strict;
use warnings;

use v5.18;

no warnings 'redefine';

my %filehandlers;



my $dos = -1;

#sub evaler {
#    my $code = shift;
#    my $oldresult = shift; # previos result
#
#    my $result = eval $code // $oldresult // '';
#    if ($@) {
#        warn $@;
#    }
#
#    return $result;
#}
#
#
#my $block_handler = \&evaler;

for our $pass (1,2) {

    for our $filename (@ARGV) {

        #print "processing $filename...";
        my $content;

        {
            local $/;
            open my $fh, '<', $filename or die $!;
            $content = <$fh>;
        }

        my $newfile;

        {
            # test first line
            if($content =~ /(\r?\n)/) {
                my $pd = $dos;
                $dos = ($1 eq "\r\n") ? 1 : 0;
                say "warning: $filename has wrong line endings! ($dos)" if $dos != $pd && $pd != -1;
            }
        }
        $content =~ s/\r\n/\n/g if $dos;

        for my $handler (values %filehandlers) {
            $handler->($filename, \$content);
        }
        
        my $dirty = 0;
        
        my $pos = 0;
        while ($content =~ m{
               (?|
               # /*<code%*/result/*>*/ /*<code>*/
               \Q/*<\E
               (.*?) # $1
               (?:
               \Q%*/\E
               (.*?) # $2
               \Q/*\E
               )?
               \Q>*/\E
               |

               # # //= code \n result \n
               # \Q//=\E\s
               # (.*?)
               # \n
               # (.*?)
               # \n
               # |

               # //=- code \n
               \Q//=-\E\s
               (.*?)
               \n
               |

               # # //== code \n result \n\n
               # \Q//==\E\s
               # (.*?)
               # \n
               # (.*?)
               # \n\s*\n
               # |

               # #if 0 //% code \n#else result \n#endif     #if 0 //% code \n#endif
               \#\s*if\s+0\s*(?://%|/\*%)
               (.*?)
               (?:
               \n\#\s*else.*?\n
               (.*?)
               )?
               \n\#\s*endif

               )
               }gsx) {

            local $SIG{__WARN__} = sub {
                if ($pass == 2) {
                    my @lines = substr($content, 0, $-[1]) =~ m/\n/g;
                    my $location = $filename . ':' . (1 + scalar(@lines)) . ": ";
                    
                    print STDERR $location . $_[0];
                }
            };

            #my $result = $block_handler->($1, $2);
            my $result = eval $1 // $2 // '';
            if ($@) {
                warn $@;
            }

            if ($pass == 2 && defined($2) && $result ne $2) {

                if ($dirty == 0) {
                    #rename($filename, $filename . ".orig");
                    open($newfile, $dos?'>:crlf':'>', $filename . '.ppnew') or die $!;
                    $dirty = 1;
                }
                
                print $newfile substr($content, $pos, $-[2] - $pos);
                
                print $newfile $result;

                $pos = $+[2];
            }
        }
        if($pass == 2 && $dirty) {
            print $newfile substr($content, $pos);
            close $newfile;
            print "$filename updated!\n";
            rename($filename . '.ppnew', $filename) or die $!;
        }
    }
}


