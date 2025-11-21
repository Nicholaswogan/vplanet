/**
  @file evolve_lsoda.c
  @brief Stiff/non-stiff adaptive integration using the liblsoda wrapper.
*/

#include "vplanet.h"

#include "liblsoda/lsoda.h"

typedef struct {
  BODY *body;
  CONTROL *control;
  SYSTEM *system;
  UPDATE *update;
  fnUpdateVariable ***fnUpdate;
  int iDir;
  int nEq;
  int *iaBody;
  int *iaVar;
  double *daAgeStart;
  double t0;
} LSODADATA;

static int fbLsodaIntegratesVar(UPDATE *update, int iBody, int iVar) {
  int iType = update[iBody].iaType[iVar][0];
  return !(iType == 0 || iType == 3 || iType == 10);
}

static int iCountLsodaVars(CONTROL *control, UPDATE *update) {
  int iBody, iVar, n = 0;
  for (iBody = 0; iBody < control->Evolve.iNumBodies; iBody++) {
    for (iVar = 0; iVar < update[iBody].iNumVars; iVar++) {
      if (fbLsodaIntegratesVar(update, iBody, iVar)) {
        n++;
      }
    }
  }
  return n;
}

static void LsodaUpdateExplicit(BODY *tmpBody, CONTROL *control, SYSTEM *system,
                                UPDATE *tmpUpdate,
                                fnUpdateVariable ***fnUpdate) {
  int iBody, iVar, iEqn;

  for (iBody = 0; iBody < control->Evolve.iNumBodies; iBody++) {
    for (iVar = 0; iVar < tmpUpdate[iBody].iNumVars; iVar++) {
      int iType = tmpUpdate[iBody].iaType[iVar][0];
      if (iType == 0 || iType == 3 || iType == 10) {
        double dValue = 0;
        for (iEqn = 0; iEqn < tmpUpdate[iBody].iNumEqns[iVar]; iEqn++) {
          tmpUpdate[iBody].daDerivProc[iVar][iEqn] =
                fnUpdate[iBody][iVar][iEqn](tmpBody, system,
                                            tmpUpdate[iBody].iaBody[iVar][iEqn]);
          dValue += tmpUpdate[iBody].daDerivProc[iVar][iEqn];
        }
        *(tmpUpdate[iBody].pdVar[iVar]) = dValue;
      }
    }
  }
}

static void LsodaBuildState(LSODADATA *data, double t, double *y) {
  EVOLVE *evolve = &(data->control->Evolve);
  int iBody;

  for (int i = 0; i < data->nEq; i++) {
    *(evolve->tmpUpdate[data->iaBody[i]].pdVar[data->iaVar[i]]) = y[i];
  }
  for (iBody = 0; iBody < data->control->Evolve.iNumBodies; iBody++) {
    double dDelta = t - data->t0;
    evolve->tmpBody[iBody].dAge =
          data->daAgeStart[iBody] + data->iDir * dDelta;
  }

  LsodaUpdateExplicit(evolve->tmpBody, data->control, data->system,
                      evolve->tmpUpdate, data->fnUpdate);
}

static int LsodaRhs(double t, double *y, double *ydot, void *user_data) {
  LSODADATA *data = (LSODADATA *)user_data;
  EVOLVE *evolve = &(data->control->Evolve);

  double dSavedTime          = evolve->dTime;
  evolve->dCurrentDt         = 0;
  evolve->dTime              = t;

  LsodaBuildState(data, t, y);
  PropertiesAuxiliary(evolve->tmpBody, data->control, data->system,
                      data->update);
  fdGetUpdateInfo(evolve->tmpBody, data->control, data->system,
                  evolve->tmpUpdate, data->fnUpdate);

  for (int i = 0; i < data->nEq; i++) {
    int iBody = data->iaBody[i];
    int iVar  = data->iaVar[i];
    int iType = evolve->tmpUpdate[iBody].iaType[iVar][0];
    double dDeriv =
          0; // Sum contributions from all processes for this primary variable
    for (int iEqn = 0; iEqn < evolve->tmpUpdate[iBody].iNumEqns[iVar]; iEqn++) {
      double dVal = evolve->tmpUpdate[iBody].daDerivProc[iVar][iEqn];
      dDeriv += (iType != 0 ? data->iDir : 1) * dVal;
    }
    ydot[i] = dDeriv;
  }

  evolve->dTime = dSavedTime;
  return 0;
}

static void LsodaApplyState(LSODADATA *data, double t, double *y) {
  EVOLVE *evolve = &(data->control->Evolve);
  int iBody, iVar;

  LsodaBuildState(data, t, y);

  for (int i = 0; i < data->nEq; i++) {
    int iBodyTmp = data->iaBody[i];
    int iVarTmp  = data->iaVar[i];
    *(data->update[iBodyTmp].pdVar[iVarTmp]) =
          *(evolve->tmpUpdate[iBodyTmp].pdVar[iVarTmp]);
  }

  for (iBody = 0; iBody < data->control->Evolve.iNumBodies; iBody++) {
    for (iVar = 0; iVar < data->update[iBody].iNumVars; iVar++) {
      int iType = data->update[iBody].iaType[iVar][0];
      if (iType == 0 || iType == 3 || iType == 10) {
        *(data->update[iBody].pdVar[iVar]) =
              *(evolve->tmpUpdate[iBody].pdVar[iVar]);
      }
    }
  }
}

void LsodaStep(BODY *body, CONTROL *control, SYSTEM *system, UPDATE *update,
               fnUpdateVariable ***fnUpdate, double *dDt, int iDir) {
  int iBody, nEq;

  if (iDir < 0) {
    fprintf(stderr,
            "ERROR: LSODA currently supports forward integration only.\n");
    exit(EXIT_INT);
  }

  nEq = iCountLsodaVars(control, update);

  double dTargetTime = control->Io.dNextOutput;
  if (control->Evolve.dStopTime < dTargetTime) {
    dTargetTime = control->Evolve.dStopTime;
  }

  if (nEq == 0) {
    *dDt                    = fabs(dTargetTime - control->Evolve.dTime);
    control->Evolve.dCurrentDt = *dDt;
    return;
  }

  BodyCopy(control->Evolve.tmpBody, body, &(control->Evolve));

  double *y     = malloc(nEq * sizeof(double));
  int *iaBody   = malloc(nEq * sizeof(int));
  int *iaVar    = malloc(nEq * sizeof(int));
  double *rtol  = malloc(nEq * sizeof(double));
  double *atol  = malloc(nEq * sizeof(double));
  double *dAges = malloc(control->Evolve.iNumBodies * sizeof(double));

  if (!y || !iaBody || !iaVar || !rtol || !atol || !dAges) {
    fprintf(stderr, "ERROR: Unable to allocate LSODA work arrays.\n");
    exit(EXIT_INT);
  }

  LSODADATA data = {.body        = body,
                    .control     = control,
                    .system      = system,
                    .update      = update,
                    .fnUpdate    = fnUpdate,
                    .iDir        = iDir,
                    .nEq         = nEq,
                    .iaBody      = iaBody,
                    .iaVar       = iaVar,
                    .daAgeStart  = dAges,
                    .t0          = control->Evolve.dTime};

  int iState = 0;
  for (iBody = 0; iBody < control->Evolve.iNumBodies; iBody++) {
    int iVar;
    data.daAgeStart[iBody] = body[iBody].dAge;
    for (iVar = 0; iVar < update[iBody].iNumVars; iVar++) {
      if (fbLsodaIntegratesVar(update, iBody, iVar)) {
        data.iaBody[iState] = iBody;
        data.iaVar[iState]  = iVar;
        y[iState]           = *(update[iBody].pdVar[iVar]);
        iState++;
      }
    }
  }

  double dRtol = control->Evolve.dLsodaRtol > 0 ? control->Evolve.dLsodaRtol
                                                : 1e-8;
  double dAtol = control->Evolve.dLsodaAtol > 0 ? control->Evolve.dLsodaAtol
                                                : 1e-10;
  for (iState = 0; iState < nEq; iState++) {
    rtol[iState] = dRtol;
    atol[iState] = dAtol;
  }

  struct lsoda_opt_t opt = {0};
  opt.rtol               = rtol;
  opt.atol               = atol;
  opt.mxstep             = control->Evolve.iLsodaMxStep;
  opt.itask              = 1;

  struct lsoda_context_t ctx = {.function = LsodaRhs,
                                .data     = &data,
                                .neq      = nEq,
                                .state    = 1,
                                .common   = NULL,
                                .opt      = NULL};

  double t    = control->Evolve.dTime;
  double tout = dTargetTime;

  lsoda_prepare(&ctx, &opt);

  int status = lsoda(&ctx, y, &t, tout);

  if (status <= 0 || ctx.state <= 0) {
    fprintf(stderr,
            "ERROR: LSODA integration failed at t=%g. istate=%d message=%s\n",
            t, ctx.state, ctx.error ? ctx.error : "none");
    lsoda_free(&ctx);
    free(y);
    free(iaBody);
    free(iaVar);
    free(rtol);
    free(atol);
    free(dAges);
    exit(EXIT_INT);
  }

  *dDt                      = fabs(t - data.t0);
  control->Evolve.dCurrentDt = *dDt;

  LsodaApplyState(&data, t, y);

  lsoda_free(&ctx);
  free(y);
  free(iaBody);
  free(iaVar);
  free(rtol);
  free(atol);
  free(dAges);
}
