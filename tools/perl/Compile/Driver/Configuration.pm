package Compile::Driver::Configuration;

use Compile::Driver::Catalog;
use Compile::Driver::Module;
use Compile::Driver::Platform;

use warnings;
use strict;


my $build = "dbg";


my %category_of_spec = qw
(
	dbg  build
	opt  build
);

sub new
{
	my $class = shift;
	
	my @specs = @_;
	
	my %self;
	
	foreach my $spec ( @specs )
	{
		my $category = $category_of_spec{ $spec };
		
		if ( !exists $self{ $category } )
		{
			$self{ $category } = $spec;
		}
		elsif ( $self{ $category } ne $spec )
		{
			die "Can't set $category=$spec; $category is already $self{ $category }\n";
		}
	}
	
	$self{ arch } = "unix";  # Set to conflict with .68k.c files with asm {}
	
	$self{ unix } = "unix";
	
	$self{ build } = $build  if !exists $self{ build };
	
	return bless \%self, $class;
}

sub platform_mask
{
	my $self = shift;
	
	my %platform = %$self;
	
	delete $platform{ build };
	
	return Compile::Driver::Platform::mask_for_values( values %platform );
}

sub target
{
	my $self = shift;
	
	my $build = $self->{build};
	
	return $build;
}

my %module_cache;

sub get_module
{
	my $self = shift;
	
	my ( $name, $mandatory ) = @_;
	
	my $optional = !$mandatory  &&  $name =~ m{^ _ }x;
	
	my $mask = $self->platform_mask;
	
	my $key = "$name $mask";
	
	return $module_cache{ $key }  if exists $module_cache{ $key };
	
	my $desc = Compile::Driver::Catalog::find_project( $name, $mask );
	
	if ( !$desc )
	{
		if ( $optional )
		{
			$desc = { NAME => $name, MEMO => { PREREQS => [] } };
		}
		else
		{
			die "No such project '$name'\n";
		}
	}
	
	return $module_cache{ $key } = Compile::Driver::Module::->new( $self, $desc );
}

sub conflicts_with
{
	my $self = shift;
	
	my ( $spec ) = @_;
	
	my $category = 'arch';
	
	my $value = $self->{ $category } or return;
	
	return $spec ne $value;
}

sub debugging
{
	my $self = shift;
	
	return $self->{ build } eq "dbg";
}

1;

