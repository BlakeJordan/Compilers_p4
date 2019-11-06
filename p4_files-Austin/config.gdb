set breakpoint pending on
set confirm off
file ./cRAPc
break cRAP::Err::report
commands
	where
end
break cRAP::ToDoError::ToDoError
commands
	where
end
break cRAP::InternalError::InternalError
commands
	where
end

define t3
  set args p3_tests/$arg0.cRAP -n --
  run
end

define t4
  set args p4_tests/$arg0.cRAP -c
  run
end
