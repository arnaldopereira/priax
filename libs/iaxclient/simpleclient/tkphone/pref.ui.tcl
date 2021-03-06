#! /bin/sh
# the next line restarts using wish8.0 \
exec wish8.0 "$0" "$@"

# interface generated by SpecTcl version 1.2 from /export/home/suhler/asterisk/iaxclient-21-oct-2004/simpleclient/tkphone/pref.ui
#   root     is the parent window for this user interface

proc pref_ui {root args} {

	# this treats "." as a special case

	if {$root == "."} {
	    set base ""
	} else {
	    set base $root
	}
    
	frame $base.frame#1

	frame $base.frame#2

	label $base.label#6 \
		-text Settings

	label $base.label#1 \
		-anchor e \
		-text {IAX server}

	entry $base.entry#1 \
		-textvariable input_server

	label $base.label#2 \
		-anchor e \
		-text {auth name}

	entry $base.entry#2 \
		-textvariable input_user \
		-width 8

	label $base.label#3 \
		-anchor e \
		-text {auth password }

	entry $base.entry#3 \
		-show * \
		-textvariable input_pass \
		-width 8

	label $base.label#4 \
		-anchor e \
		-text {phone extension}

	entry $base.entry#4 \
		-textvariable input_ext \
		-width 5

	label $base.label#7 \
		-text {preferred codec}

	radiobutton $base.radiobutton#1 \
		-text gsm \
		-value gsm \
		-variable input_preferred_codec

	radiobutton $base.radiobutton#2 \
		-text ulaw \
		-value ulaw \
		-variable input_preferred_codec

	radiobutton $base.radiobutton#3 \
		-text speex \
		-value speex \
		-variable input_preferred_codec

	button $base.button#4 \
		-command pref_ok \
		-text ok

	button $base.button#5 \
		-command pref_cancel \
		-text cancel


	# Add contents to menus

	# Geometry management

	grid $base.frame#1 -in $root	-row 7 -column 1  \
		-columnspan 2
	grid $base.frame#2 -in $root	-row 6 -column 2 
	grid $base.label#6 -in $root	-row 1 -column 1  \
		-columnspan 2
	grid $base.label#1 -in $root	-row 2 -column 1  \
		-sticky ew
	grid $base.entry#1 -in $root	-row 2 -column 2  \
		-sticky w
	grid $base.label#2 -in $root	-row 3 -column 1  \
		-sticky ew
	grid $base.entry#2 -in $root	-row 3 -column 2  \
		-sticky w
	grid $base.label#3 -in $root	-row 4 -column 1  \
		-sticky ew
	grid $base.entry#3 -in $root	-row 4 -column 2  \
		-sticky w
	grid $base.label#4 -in $root	-row 5 -column 1  \
		-sticky ew
	grid $base.entry#4 -in $root	-row 5 -column 2  \
		-sticky w
	grid $base.label#7 -in $root	-row 6 -column 1  \
		-sticky e
	grid $base.radiobutton#1 -in $base.frame#2	-row 1 -column 1 
	grid $base.radiobutton#2 -in $base.frame#2	-row 1 -column 2 
	grid $base.radiobutton#3 -in $base.frame#2	-row 1 -column 3 
	grid $base.button#4 -in $base.frame#1	-row 1 -column 1 
	grid $base.button#5 -in $base.frame#1	-row 1 -column 2 

	# Resize behavior management

	grid rowconfigure $base.frame#2 1 -weight 0 -minsize 9 -pad 0
	grid columnconfigure $base.frame#2 1 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $base.frame#2 2 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $base.frame#2 3 -weight 0 -minsize 30 -pad 0

	grid rowconfigure $root 1 -weight 0 -minsize 30 -pad 0
	grid rowconfigure $root 2 -weight 0 -minsize 9 -pad 0
	grid rowconfigure $root 3 -weight 0 -minsize 5 -pad 0
	grid rowconfigure $root 4 -weight 0 -minsize 6 -pad 0
	grid rowconfigure $root 5 -weight 0 -minsize 11 -pad 0
	grid rowconfigure $root 6 -weight 0 -minsize 5 -pad 0
	grid rowconfigure $root 7 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $root 1 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $root 2 -weight 0 -minsize 30 -pad 0

	grid rowconfigure $base.frame#1 1 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $base.frame#1 1 -weight 0 -minsize 30 -pad 0
	grid columnconfigure $base.frame#1 2 -weight 0 -minsize 30 -pad 0
# additional interface code
# end additional interface code

}


# Allow interface to be run "stand-alone" for testing

catch {
    if [info exists embed_args] {
	# we are running in the plugin
	pref_ui .
    } else {
	# we are running in stand-alone mode
	if {$argv0 == [info script]} {
	    wm title . "Testing pref_ui"
	    pref_ui .
	}
    }
}
