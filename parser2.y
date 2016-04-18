primary_expression
: T_THIS
| identifier
| literal
| '(' expression ')'
;

member_expression
: primary_expression
| member_expression '[' expression ']'
| member_expression '.' identifier
| T_NEW member_expression arguments
;

new_expression
: member_expression
| T_NEW new_expression
;

argument_list
: assignment_expression
| argument_list ',' assignment_expression
;










