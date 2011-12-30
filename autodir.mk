.PHONY : FORCE
# This one here allows automatic directory creation
# All that is required is the secondary expansion of the prerequisities.
# And all the object files shall also be dependant, 'order only', on their
# containting directories + '/.' suffix.
.PRECIOUS: %/.
%/.:
	@$(ECHO_MD) $*
	$(NOECHO)mkdir -p $@

# If a rule has no prerequisites or commands, and the target of the rule is a nonexistent file, 
# then `make' imagines this target to have been updated whenever its rule is run. This implies 
# that all targets depending on this one will always have their commands run.
# In short - this will trigger regenaration of MD5 checksum for all the source and header
# files, regardless of their timestamp.
FORCE: ;

