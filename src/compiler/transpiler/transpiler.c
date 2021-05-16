#include "transpiler.h"
#include "../io/io.h"
#include "../io/log.h"

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

transpiler_T* initTranspiler(const char* target, const char* cachePath)
{
    transpiler_T* tp = malloc(sizeof(struct TRANSPILER_STRUCT));
    tp->cachePath = cachePath;
    tp->target = target;

    // the first section of the source file where libraries get included
    tp->inclSection = malloc(sizeof(char));
    tp->inclSection[0] = '\0';

    // the second section of the source file where typedefs are located
    tp->typeSection = malloc(sizeof(char));
    tp->typeSection[0] = '\0';

    // the third section where functions and globals are defined, because order does not matter in CSpydr
    tp->defSection = malloc(sizeof(char));
    tp->defSection[0] = '\0';

    // the last part where the actual source code lives
    tp->implSection = malloc(sizeof(char));
    tp->implSection[0] = '\0';

    return tp;
}

void freeTranspiler(transpiler_T* tp)
{
    free(tp->inclSection);
    free(tp->typeSection);
    free(tp->defSection);
    free(tp->implSection);

    free(tp);
}

void transpile(ASTProgram_T* ast, char* target)
{
    //FIXME: get the real cache path
    const char* cachePath = ".cache";
    transpiler_T* tp = initTranspiler(target, cachePath);
    generateCCode(tp, ast);

    // merge the 3 code sections to one source file to compile
    const char* CSrcTmp = "//transpiled by the CSpydr compiler\n%s\n%s\n%s\n%s";
    char* CSrc = calloc(strlen(CSrcTmp) 
                      + strlen(tp->inclSection) 
                      + strlen(tp->typeSection)
                      + strlen(tp->defSection) 
                      + strlen(tp->implSection) 
                      + 1, sizeof(char));
    sprintf(CSrc, CSrcTmp, tp->inclSection, tp->typeSection, tp->defSection, tp->implSection);

    // Get the temporary path for the C source code for compilation
    struct stat st = {0};
    if (stat(cachePath, &st) == -1) {
        mkdir(cachePath, 0700);
    }

    const char* CTargetPathTmp = "%s/%s.c";
    char* CTargetPath = calloc(strlen(CTargetPathTmp) 
                             + strlen(cachePath)
                             + strlen(target)
                             + 1, sizeof(char));
    sprintf(CTargetPath, CTargetPathTmp, cachePath, target);
    // write the C code to the file
    writeFile(CTargetPath, CSrc);

    // compile the file and finish
    compile(tp);
    free(CTargetPath);
    free(CSrc);
    freeTranspiler(tp);
}

void compile(transpiler_T* tp)
{
    const char* gccTemplate = "gcc -Wall -fPIC -O3 %s/%s.c -o %s 2>&1";
    char* gccCmd = calloc(strlen(gccTemplate) 
                        + strlen(tp->cachePath) 
                        + strlen(tp->target) * 2 
                        + 1, sizeof(char));
    sprintf(gccCmd, gccTemplate, tp->cachePath, tp->target, tp->target);

    char* compilerFeedback = sh(gccCmd);
    LOG_OK_F(COLOR_RESET "%s", compilerFeedback);
    free(compilerFeedback);

    free(gccCmd);
}