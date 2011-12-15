%{
// In head
%}

expr1 [ab]*abb

%%

{expr1}  {
    printf("done");
}

%%

// In the middle of programme
