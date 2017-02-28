use strict;
use warnings;

use v5.18;

no warnings 'redefine';

my %filehandlers;




for my $pass (1,2) {

    for our $filename (@ARGV) {

        #print "processing $filename...";
        my $content;

        {
            local $/;
            open my $fh, '<', $filename or die $!;
            $content = <$fh>;
        }

        my $newfile;

        my $dos = 0;
        my @crlfs = $content =~ /\r\n/g;
        my @lfs = $content =~ /\n/g;
        $dos = 1 if (scalar @crlfs) == (scalar @lfs);
        #print "detected CRLF\n" if $dos;
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
            
            #print "$1\n$2\n\n";
            
            local $SIG{__WARN__} = sub {
                if ($pass == 2) {
                    my @lines = substr($content, 0, $-[1]) =~ m/\n/g;
                    my $location = $filename . ':' . (1 + scalar(@lines)) . ": ";
                    
                    print STDERR $location . $_[0];
                }
            };

            my $result = eval $1;
            if ($@) {
                warn $@;
            }

            $result = '' unless defined($result);
            
            if ($pass == 2 && defined($2) && $result ne $2) {

                if ($dirty == 0) {
                    #rename($filename, $filename . ".orig");
                    open($newfile, $dos?'>:crlf':'>', $filename . '.ppnew');
                    $dirty = 1;
                }
                
                print $newfile substr($content, $pos, $-[2] - $pos);
                
                print $newfile $result;

                #print $newfile substr($content, $+[2], $+[0] - $+[2]); # /*>*/
                $pos = $+[2];
            }
        }
        if($pass == 2 && $dirty) {
            print $newfile substr($content, $pos);
            print "$filename updated!\n";
            rename($filename . '.ppnew', $filename);
        }
    }
}


