void yylex()
{
    if (yyin == NULL)
        yyin = stdin;

    // look back buffer
    char buffer[512];
    int count = 0;
    int success = -1;

    int state = 0;
    char ch;
    for (ch = fgetc(yyin); ch != EOF;) {
        yytext[yyleng++] = ch;

        state = lexi_DTran[state][(int)ch];

        // when found one match we keep chars into lookback buffer 
        // until there is no longer match
        if (success > -1)
            buffer[count++] = ch;

        if (count > 512) {
            fprintf (stderr, "Panic, look back buffer is full!\n");
            exit(1);
        }

        if (state == -1) {
            // go back to last match
            if (success > -1) {
                // get right yyleng
                yyleng -= count;
                yytext[yyleng] = 0;

                // give back chars
                while(count > 0)
                    ungetc(buffer[--count], yyin);

                lexi_Action(success);
            } else {
                fprintf (stderr, "Invalid: %c\n", ch);
            }

            state = 0;
            count = 0;
            success = -1;

            // reset yyleng and yytext
            yyleng = 0;
            yytext[0] = 0;
        } else if (lexi_Finished[state]) {
            // found match, wait for longer match
            success = state;
            // and discard old lookback buffer
            count = 0;
        }

        ch = fgetc(yyin);
    }
}

void lexi_Action(int lexi_i) {
    switch (lexi_i) {
