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
                lexi_Action(success);

                // give back chars
                while(count > 0)
                    ungetc(buffer[--count], yyin);
            } else {
                fprintf (stderr, "Invalid: %c\n", ch);
            }

            state = 0;
            success = -1;
        } else if (lexi_Finished[state]) {
            // found match, wait for longer match
            success = state;
            count = 0;
        }

        ch = fgetc(yyin);
    }
}

void lexi_Action(int lexi_i) {
    switch (lexi_i) {
