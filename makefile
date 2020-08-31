#Our main target : 'compute'
compute : compute.c expression_evaluator.o info_fetch.o
	gcc -g compute.c expression_evaluator.o info_fetch.o -o compute -lm

#Dependencies
expression_evaluator.o : expression_evaluator.c expression_evaluator.h
	gcc -c -g expression_evaluator.c -o expression_evaluator.o
	
info_fetch.o : info_fetch.c info_fetch.h
	gcc -c -g info_fetch.c -o info_fetch.o

#Phony region

.PHONY : install
.PHONY : uninstall

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = ${INSTALL} -m 644

prefix = /usr/local
bindir = $(prefix)/bin

application = compute
help_file = help_doc
#Dummy file names
#What's the purpose of the 'dest_dir_addr_file' variable? This variable will hold the name of a file, which will be responsible for recording the address of the destination directory. This destination directory will be the place where the installation will happen. Thus name of the file responsible for holding the destination directory address is kept as '.dest_dir_addr'. Note the presence of the '.' character in front of the file name. This allows the file to be hidden by default from the user's eyes, so that they do not accidently modify it's contents. Thus the file '.dest_dir_addr' will be created everytime installation of the application actually takes place, so that the installation directory address is recorded in it. Now, this file will be deleted everytime the application is uninstalled. Hence, in the absence of this file the makefile will treat the application as 'uninstalled' OR absent. And in the presence of this file, the application will be treated as installed by the makefile. NOTE : DO NOT MODIFY THE '.dest_dir_addr' ON YOUR OWN.
dest_dir_addr_file = .dest_dir_addr
#The following two dummy files are used for handling the installation of the application. With thier help, the makefile comes to know whether OR not the application needs to be re-installed OR updated, if the 'make install' command is invoked consecutively on the shell.
done_app = .done_app
done_help = .done_help

#If the variable DESTDIR is empty( i.e. the user has not provided the value for it), then $(bin_dir) should be used to initialise the default value of dest_dir. Otherwise, dest_dir should be initialised with the value of $(DESTDIR).
dest_dir = $(if $(DESTDIR), $(DESTDIR), $(bindir))
#Note that, making dest_dir a target specific variable for the install recipe poses the following issue :
#The install recipe is unable to differentiate whether OR not the application is installed (for some reason I cannot understand). This implies that, everytime the install recipe is invoked, the application is reinstalled
#Check if the directory held in dest_dir actually exists
dir_status = $(shell [ -d $(dest_dir) ] && echo 1 || echo 0 )

#Perform installation

install : check_dir $(dest_dir)/$(application) $(dest_dir)/$(help_file)
	@echo 'Performing installation...'
	@[ -f $(done_app) ] && rm $(done_app) || echo "Installed 'terminal_calculator' copy is up to date. No further changes/installation needed."
	@[ -f $(done_help) ] && rm $(done_help) || echo "Installed help file copy is up to date. No further changes/installation needed."
	@echo 'Installation procedure finished. Enjoy using terminal_calculator!'
	
#If the directory held in dest_dir actually exists, then print the directory otherwise exit after printing the error message
check_dir : 
ifeq ($(dir_status), 1)
	@echo "Destination is set to '" $(dest_dir) "'."
else
	$(error $(dest_dir) is not a directory)
endif
	
$(dest_dir)/$(application) : $(application)
#If the installed application copy at the destination is older than the source copy, then enter here
	@$(INSTALL) $(application) $(dest_dir)
#Update the destination directory address held in the '.dest_dir_addr' file, everytime the application is installed
	$(file > $(dest_dir_addr_file),$(dest_dir))
	@touch $(done_app)

$(dest_dir)/$(help_file) : $(help_file)
#If the installed help_file copy at the destination is older than the source copy, then enter here
	@$(INSTALL_DATA) $(help_file) $(dest_dir)
	@touch $(done_help)

#Perform uninstallation

#Read the installation directory address into a target-specific variable, iff the file which holds the installation directory address exists
uninstall : install_dir != [ -f $(dest_dir_addr_file) ] && cat $(dest_dir_addr_file)

uninstall :
	@echo 'Performing uninstallation...'
#Note that the conditional constructs like 'ifeq..' construct is a part of 'make' and not 'shell'. Hence, these constructs should never be written with a 'TAB' character sitting write in front of them. Why? Because every line within a recipe section that begins with a 'TAB' character is sent to the shell for execution. And shell doesn't know shit about make's condtional constructs.
#'wildcard' function checks if the given type of file exists OR not. Generally, the wildcard function, takes in a pattern (regex type) as input and checks if those type of files do exist. If they exist, then the complete names of all those files which match the pattern are returned by this 'wildcard' function as output. Note that '$' represents empty string in make. Therefore, if the file with the requested name pattern doesn't exist, then wildcard function will return empty string. To match this empty string we have the '$' symbol.
ifneq "$(wildcard $(dest_dir_addr_file))" "$"
	@echo "Uninstalling 'terminal_calculator' from '"$(install_dir)"' ..."
	@rm $(install_dir)/$(application)
	@rm $(install_dir)/$(help_file)
	@rm $(dest_dir_addr_file)
	@echo "Done. Thank you for using 'terminal_calculator'!"
else
	@echo "Sorry couldn't perform uninstallation.\nLooks like either 'terminal_calculator' is already uninstalled OR it never existed to begin with"
endif	
