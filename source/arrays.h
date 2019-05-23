# ifdef ARRAY
#   ifndef ARRAYS_H
#   define ARRAYS_H
/* THE WONDERFUL TEXT GRAPHIC ARRAYS FROM THE OLD OLD GB CLIENT */

/* 16 Novaes of 11(+1 for null)x7 */
static char *Novae[16][7] = {
				"           ",
				"           ",
				"           ",
				"    %*     ",
				"           ",
				"           ",
				"           ",

				"           ",
				"           ",
				"     %     ",
				"    %*%    ",
				"     %     ",
				"           ",
				"           ",

				"           ",
				"           ",
				"     %%    ",
				"   %%*%%   ",
				"    %%     ",
				"           ",
				"           ",

				"           ",
				"           ",
				"     %%%   ",
				"    %*%%%  ",
				"     %%%   ",
				"           ",
				"           ",

				"           ",
				"           ",
				"    %%%    ",
				"   %%*%%%  ",
				"    %%%%   ",
				"           ",
				"           ",

				"           ",
				"     %     ",
				"   %%%%    ",
				"  %%%*%%%  ",
				"   %%%%%   ",
				"           ",
				"           ",

				"           ",
				"     %     ",
				"   %%%%%   ",
				"  %% *%%%  ",
				"   %%%%%   ",
				"     %     ",
				"           ",

				"           ",
				"     %     ",
				"  %%%%%%%  ",
				" %%% * %%% ",
				"   %%%%%   ",
				"     %%    ",
				"           ",

				"           ",
				"    %%%    ",
				"  %%%%%%%  ",
				" %%% * %%% ",
				"  %%% %%%  ",
				"    %%%    ",
				"           ",

				"           ",
				"    %%%%   ",
				"  %%% %%%  ",
				" %%% * %%% ",
				"  %%% %%%% ",
				"   %%%%%%  ",
				"     %%    ",

				"           ",
				"    %%%%   ",
				"  %%% %%%  ",
				" %%% * %%% ",
				"  %%%  %%% ",
				"   %%%%%%  ",
				"     %%    ",

				"     %%    ",
				"   %%%%%%  ",
				"  %%   %%% ",
				" %%% *  %%%",
				" %%%%  %%% ",
				"  %%%%%%%% ",
				"     %%%   ",

				"    %%%%   ",
				"  %%%%%%%% ",
				" %%%  % %%%",
				" %%  *   %%",
				" %%%    %%%",
				"  %%%%%% % ",
				"   % %%%   ",

				"   %%% %%  ",
				" %%% %% %% ",
				"%%      %%%",
				"%%%  * % %%",
				" % %     %%",
				" %%% % % % ",
				"  %%%%%%%  ",

				"%  % % %%  ",
				"      %  % ",
				"%%      % %",
				"%    *   % ",
				" %        %",
				" %     % % ",
				"  % %%%    ",

				"    %  %   ",
				"          %",
				"%          ",
				"     *    %",
				"%          ",
				" %        %",
				"   %   % % "
};


/* 8 mirrors of size 9(+1 for null)x5 */
static char *Mirror[8][5] = {

				"         ",
				"         ",
				"\\=======/",
				"         ",
				"         ",

				"   |     ",
				"   \\     ",
				"   \\\\    ",
				"    \\\\   ",
				"     \\\\__",

				"     /   ",
				"    ||   ",
				"    ||   ",
				"    ||   ",
				"     \\   ",

				"       __",
				"     //  ",
				"    //   ",
				"    /    ",
				"   |     ",

				"         ",
				"         ",
				"/=======\\",	/* cc thinks this is a quoted
						 * " w/o the \ */
				"         ",
				"         ",

				"__       ",
				"  \\\\     ",	/* weird stuff again */
				"   \\\\    ",
				"    \\\\   ",
				"     |   ",

				"   \\     ",
				"   ||    ",
				"   ||    ",
				"   ||    ",
				"   /     ",

				"     |   ",
				"    //   ",
				"   //    ",
				"__//     ",
				"         "

};
#   endif
# endif