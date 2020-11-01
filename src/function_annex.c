#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <dirent.h>

#include "raler.c"
#include "godunov.h"
#include "parameters.c"

#define CHEMIN_MAX 512
#define BORDER 0.1

//-----------------------------------------------------------------------------
// Manage of parameters
//-----------------------------------------------------------------------------

void godunov_parameters(godunov * pgd, char * option){
    // Distrib the function pointers for godunov pgd depending on option
    // If option do not exist, a alert message is send

    if (strcmp(option,"transport_1d_1") == 0){
        pgd->pfluxnum = fluxnum_trans1;
        pgd->plambda_ma = lambda_ma_trans1;
        pgd->pboundary_spatial = boundary_spatial_trans1;
        pgd->pboundary_temporal_left = boundary_temporal_left_trans1;
        pgd->pboundary_temporal_right = boundary_temporal_right_trans1;

        if (pgd->keept_solexacte){
            pgd->psolexacte = solexacte_trans1;
        }
    }
    else if (strcmp(option,"burgers_1d_1") == 0){
        pgd->pfluxnum = fluxnum_burgers;
        pgd->plambda_ma = lambda_ma_burgers;
        pgd->pboundary_spatial = boundary_spatial_burgers1;
        pgd->pboundary_temporal_left = boundary_temporal_left_burgers1;
        pgd->pboundary_temporal_right = boundary_temporal_right_burgers1;

        if (pgd->keept_solexacte){
            pgd->psolexacte = solexacte_burgers1;
        }
    }
    else if (strcmp(option,"burgers_1d_2") == 0){
        pgd->pfluxnum = fluxnum_burgers;
        pgd->plambda_ma = lambda_ma_burgers;
        pgd->pboundary_spatial = boundary_spatial_burgers2;
        pgd->pboundary_temporal_left = boundary_temporal_left_burgers2;
        pgd->pboundary_temporal_right = boundary_temporal_right_burgers2;

        if (pgd->keept_solexacte){
            pgd->psolexacte = solexacte_burgers2;
        }
    }
    else {
        raler(0,"The option \"%s\" dot not exist", option);
    }
    
}

void godunov_error_parameters(godunov_error * pgderr, char * option_error){
    // Distrib the function pointers for godunov_error pgderr depending on
    // option
    // If option do not exist, a alert message is send

    if (option_error = "norm_L1"){
        pgderr->perror = norm_L1;
    }
    else if (option_error = "norm_L2"){
        pgderr->perror = norm_L2;
    }
    else if (option_error = "norm_inf")
        pgderr->perror = norm_inf;
    else {
        raler(0,"The option \"%s\" dot not exist", option_error);
    }
}


//-----------------------------------------------------------------------------
// Fonction output
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Fonction pour créer les fichiers outputs godunov

void gd_create_parameters(godunov * pgd){
    // Create file parameters for godunov object pgd

    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, pgd->complete_path_output);
    strcat(name_file, "/");
    strcat(name_file, "parameters");
    
    FILE *fic = fopen(name_file, "w");
    
    fprintf(fic, "Parametres %s :\n\n", pgd->name_file);
    fprintf(fic, "keept_solexacte %d\n", pgd->keept_solexacte);
    fprintf(fic, "N %d\n", pgd->N);
    fprintf(fic, "m %d\n", pgd->m);
    fprintf(fic, "dx %f\n", pgd->dx);
    fprintf(fic, "tmax %f\n", pgd->tmax);
    fprintf(fic, "xmin %f\n", pgd->xmin);
    fprintf(fic, "xmax %f\n", pgd->xmax);
    fprintf(fic, "cfl %f\n", pgd->cfl);
    fprintf(fic, "time %ld\n", pgd->time);
    
    fclose(fic);
    free(name_file);
}

void gd_create_plot(godunov * pgd){
    // Create file plot.dat for godunov object pgd
    // Give exact solution if pgd->keept_solution=1

    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, pgd->complete_path_output);
    strcat(name_file, "/");
    strcat(name_file, "plot.dat");

    FILE *fic = fopen(name_file, "w");

    if (pgd->keept_solexacte){
        for (int i = 0; i < pgd->N+2; i++){
            fprintf(fic, "%f %f %f\n", pgd->xi[i], pgd->un[i], pgd->sol[i]);
        }
    }
    else {
        for (int i = 0; i < pgd->N+2; i++){
            fprintf(fic, "%f %f\n", pgd->xi[i], pgd->un[i]);
        }
    }

    fclose(fic);
    free(name_file);
}

void gd_create_plots(godunov * pgd){
    // Create the ploti.dat with i=pgd->len_U-1

    char * name_plot = malloc(CHEMIN_MAX);
    strcpy(name_plot, pgd->complete_path_output);
    char * inter = malloc(CHEMIN_MAX);
    sprintf(inter, "/plots/plot%d.dat", pgd->len_U);
    strcat(name_plot, inter);

    free(inter);

    FILE * fic = fopen(name_plot, "w");

    for (int i=0; i<pgd->N+2; i++){
        fprintf(fic, "%f %f \n", pgd->xi[i], pgd->un[i]);
    }

    fclose(fic);
    free(name_plot);
}

void gd_create_execute_gnu(godunov * pgd){
    // Create and execute the gnuplot plotcom.gnu for godunov object pgd

    // Create of plotcom.gnu
    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, pgd->complete_path_output);
    strcat(name_file, "/");
    strcat(name_file, "plotcom.gnu");

    FILE *fic = fopen(name_file, "w");
    
    fprintf(fic, "set terminal pngcairo\n");
    fprintf(fic, "set output \'%s/graphe.png\'\n\n", pgd->complete_path_output);
    fprintf(fic, "set title \"Resolution de %s tmax=%f\"\n", pgd->option_godunov, pgd->tmax);
    fprintf(fic, "set xlabel \"x\"\n");
    fprintf(fic, "set ylabel \"u\"\n\n");
    fprintf(fic, "stats \'%s/plot.dat\' using 1:2 nooutput\n", pgd->complete_path_output);
    fprintf(fic, "set xrange [STATS_min_x:STATS_max_x]\n");
    fprintf(fic, "set yrange [STATS_min_y - %f * (STATS_max_y-STATS_min_y): STATS_max_y + %f * (STATS_max_y-STATS_min_y)]\n\n", BORDER, BORDER);
    fprintf(fic, "plot \'%s/plot.dat\' using 1:2 title \"solution numerique\" w lp pt 0", pgd->complete_path_output);
    if (pgd->keept_solexacte){
        fprintf(fic, ", \'%s/plot.dat\' using 1:3 title \"soluton exacte\" w lp pt 0", pgd->complete_path_output);
    }
    
    fclose(fic);
    free(name_file);

    // Execution de la commande gnuplot
    char * name_command = malloc(CHEMIN_MAX);
    strcpy(name_command, "gnuplot ");
    strcat(name_command, pgd->complete_path_output);
    strcat(name_command, "/");
    strcat(name_command, "plotcom.gnu");
    
    int status = system(name_command);
    assert(status == EXIT_SUCCESS);

    free(name_command);
}

void gd_create_animation(godunov * pgd){
    // Create and execute the gnuplot plotcom.gnu for godunov object pgd

    // Create of folder animation
    char * path_animation = malloc(CHEMIN_MAX);
    strcpy(path_animation, pgd->complete_path_output);
    strcat(path_animation, "/animation");

    mkdir(path_animation, ACCESSPERMS);

    free(path_animation);

    // Create of plotcom.gnu
    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, pgd->complete_path_output);
    strcat(name_file, "/plotcom.gnu");

    FILE *fic = fopen(name_file, "w");
    
    fprintf(fic, "set terminal pngcairo\n\n");
    fprintf(fic, "stats \'%s/plots/plot1.dat\' using 1:2 nooutput\n\n", pgd->complete_path_output);
    fprintf(fic, "Xmin = STATS_min_x\n");
    fprintf(fic, "Xmax = STATS_max_x\n");
    fprintf(fic, "Ymin = STATS_min_y - %f * (STATS_max_y - STATS_min_y)\n", BORDER);
    fprintf(fic, "Ymax = STATS_max_y + %f * (STATS_max_y - STATS_min_y)\n\n", BORDER);

    fprintf(fic, "do for [i=0:%d] {\n", pgd->len_U-1);
    fprintf(fic, "\tset output sprintf(\'%s/animation/graphe%%d.png\',i) \n\n", pgd->complete_path_output);
    fprintf(fic, "\tset title sprintf(\"Animation de %s len_U=%%d\",i) \n", pgd->option_godunov);
    fprintf(fic, "\tset key off\n\n");
    fprintf(fic, "\tset xlabel \"x\" \n");
    fprintf(fic, "\tset ylabel \"u\" \n\n");
    fprintf(fic, "\tset xrange [Xmin:Xmax] \n");
    fprintf(fic, "\tset yrange [Ymin:Ymax] \n\n");
    fprintf(fic, "\tplot sprintf(\'%s/plots/plot%%d.dat\', i) using 1:2 w lp pt 0 \n", pgd->complete_path_output);
    fprintf(fic, "}");
    
    fclose(fic);
    free(name_file);
    
    // Execution de la commande gnuplot
    char * name_command = malloc(CHEMIN_MAX);
    strcpy(name_command, "gnuplot ");
    strcat(name_command, pgd->complete_path_output);
    strcat(name_command, "/plotcom.gnu");
    
    int status = system(name_command);
    assert(status == EXIT_SUCCESS);
    
    free(name_command);

    // Creation of video
    name_command = malloc(CHEMIN_MAX);
    sprintf(name_command,
                "mencoder mf://%s/animation/*.png -mf w=800:h=600:fps=25:type=png -ovc lavc -lavcopts vcodec=mpeg4 -oac copy -o %s/animation.avi",
                pgd->complete_path_output, pgd->complete_path_output);
    
    status = system(name_command);
    assert(status == EXIT_SUCCESS);
    
    free(name_command);

    printf("Fin create of animation\n");
}


//-----------------------------------------------------------------------------
// Fonction pour créer les fichiers outputs godunov_error

void gderr_create_parameters(godunov_error * pgderr, char * output_path){
    // Create parameters for godunov_error object pgderr

    char * name_output = malloc(CHEMIN_MAX);
    strcpy(name_output, output_path);
    strcat(name_output, "/");
    strcat(name_output, "parameters");

    FILE *fic = fopen(name_output, "w");

    fprintf(fic, "Parametres %s:\n\n", output_path);
    fprintf(fic, "len_liste_N %d\n", pgderr->len_liste_N);
    fprintf(fic, "liste_N ");
    for (int i=0; i<pgderr->len_liste_N; i++){
        fprintf(fic, "%d ", pgderr->liste_N[i]);
    }
    fprintf(fic, "\n");
    fprintf(fic, "m %d\n", pgderr->m);
    fprintf(fic, "xmin %f\n", pgderr->xmin);
    fprintf(fic, "xmax %f\n", pgderr->xmax);
    fprintf(fic, "cfl %f\n", pgderr->cfl);

    fclose(fic);
    free(name_output);
}

void gderr_create_plot(godunov_error * pgderr, char * output_path){
    // Create plot.dat for godunov_error object pgderr

    char * name_output = malloc(CHEMIN_MAX);
    strcpy(name_output, output_path);
    strcat(name_output, "/");
    strcat(name_output, "plot.dat");

    FILE *fic = fopen(name_output, "w");

    for (int i = 0; i < pgderr->len_liste_N; i++){

        fprintf(fic, "%d %f %ld\n", pgderr->liste_N[i], pgderr->liste_error[i], pgderr->liste_time[i]);

    }

    fclose(fic);
    free(name_output);
}

void gderr_create_execute_gnu(godunov_error * pgderr, char * output_path){
    // Create and execute the gnuplot script plotcom.gnu for godunov_error object pgderr

    char * name_file = malloc(CHEMIN_MAX);
    strcpy(name_file, output_path);
    strcat(name_file, "/");
    strcat(name_file, "plotcom.gnu");

    FILE *fic = fopen(name_file, "w");
    
    fprintf(fic, "set terminal pngcairo\n\n");
    fprintf(fic, "# Graphic of error\n");
    fprintf(fic, "set output \'%s/error.png\'\n\n", output_path);
    fprintf(fic, "set title \"Erreur du schéma de Godunov pour %s en %s\"\n", pgderr->option_godunov, pgderr->option_error);
    fprintf(fic, "set xlabel \"N\"\n");
    fprintf(fic, "set ylabel \"error\"\n\n");
    fprintf(fic, "set logscale x 10\n");
    fprintf(fic, "stats \'%s/plot.dat\' using 1:2 nooutput\n", output_path);
    fprintf(fic, "set xrange [STATS_min_x:STATS_max_x]\n");
    fprintf(fic, "set yrange [0: STATS_max_y + %f * (STATS_max_y-STATS_min_y)]\n\n", BORDER);
    fprintf(fic, "plot \'%s/plot.dat\' using 1:2 title \"error\" w lp\n\n", output_path);
    fprintf(fic, "reset\n\n");
    fprintf(fic, "# Graphic of time\n");
    fprintf(fic, "set output \'%s/time.png\'\n\n", output_path);
    fprintf(fic, "set title \"Duree\"\n");
    fprintf(fic, "set xlabel \"N\"\n");
    fprintf(fic, "set ylabel \"time (s)\"\n\n");
    //fprintf(fic, "set logscale x 10\n");
    fprintf(fic, "set autoscale y\n");
    fprintf(fic, "stats \'%s/plot.dat\' using 1:3 nooutput\n", output_path);
    fprintf(fic, "set xrange [STATS_min_x:STATS_max_x]\n");
    //fprintf(fic, "set yrange [0: 1 + STATS_max_y + %f * (STATS_max_y-STATS_min_y)]\n\n", BORDER);
    fprintf(fic, "plot \'%s/plot.dat\' using 1:3 title \"time\" w lp", output_path);
    
    fclose(fic);
    free(name_file);

    // Execution de la commande gnuplot
    char * name_command = malloc(CHEMIN_MAX);
    strcpy(name_command, "gnuplot ");
    strcat(name_command, output_path);
    strcat(name_command, "/");
    strcat(name_command, "plotcom.gnu");
    
    int status = system(name_command);
    assert(status == EXIT_SUCCESS);

    free(name_command);
}