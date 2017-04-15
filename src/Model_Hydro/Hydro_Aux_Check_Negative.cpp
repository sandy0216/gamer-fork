#include "Copyright.h"
#include "GAMER.h"

#if ( MODEL == HYDRO )




//-------------------------------------------------------------------------------------------------------
// Function    :  Hydro_Aux_Check_Negative
// Description :  Check if there is any cell with negative density or pressure
//
// Parameter   :  lv       : Targeted refinement level
//                Mode     : 1 : Check negative density
//                           2 : Check negative pressure (and entropy when DUAL_ENERGY == DE_ENTROPY)
//                           3 : Both
//                comment  : You can put the location where this function is invoked in this string
//-------------------------------------------------------------------------------------------------------
void Hydro_Aux_Check_Negative( const int lv, const int Mode, const char *comment )
{

// check
   if ( lv < 0  ||  lv >= NLEVEL )  Aux_Error( ERROR_INFO, "incorrect parameter %s = %d !!\n", "lv", lv );
   if ( Mode < 1  ||  Mode > 3 )    Aux_Error( ERROR_INFO, "incorrect parameter %s = %d !!\n", "Mode", Mode );


   const real Gamma_m1        = GAMMA - (real)1.0;
   const bool CheckMinPres_No = false;

   int  Pass = true;
   real Pres, Fluid[NCOMP_TOTAL];

#  if ( DUAL_ENERGY == DE_ENTROPY )
   const real CorrPres_No = -__FLT_MAX__;    // set minimum pressure to an extremely negative value
#  endif


   for (int TargetRank=0; TargetRank<MPI_NRank; TargetRank++)
   {
      if ( MPI_Rank == TargetRank )
      {
         for (int PID=0; PID<amr->NPatchComma[lv][1]; PID++)
         for (int k=0; k<PATCH_SIZE; k++)
         for (int j=0; j<PATCH_SIZE; j++)
         for (int i=0; i<PATCH_SIZE; i++)
         {
            for (int v=0; v<NCOMP_TOTAL; v++)   Fluid[v] = amr->patch[ amr->FluSg[lv] ][lv][PID]->fluid[v][k][j][i];

#           if ( DUAL_ENERGY == DE_ENTROPY )
            Pres = CPU_DensEntropy2Pres( Fluid[DENS], Fluid[ENTROPY], Gamma_m1, CorrPres_No );
#           else
            Pres = CPU_GetPressure( Fluid[DENS], Fluid[MOMX], Fluid[MOMY], Fluid[MOMZ], Fluid[ENGY],
                                    Gamma_m1, CheckMinPres_No, NULL_REAL );
#           endif

            if ( Mode == 1  ||  Mode == 3 )
            {
               if ( Fluid[DENS] <= (real)0.0 )
               {
                  if ( Pass )
                  {
                     Aux_Message( stderr, "\"%s\" : <%s> FAILED at level %2d, Time = %13.7e, Step = %ld !!\n",
                                  comment, __FUNCTION__, lv, Time[lv], Step );
                     Aux_Message( stderr, "%4s  %7s  %19s  %10s  %21s  %21s  %21s  %21s  %21s",
                                  "Rank", "PID", "Patch Corner", "Grid ID", "Dens", "MomX", "MomY", "MomZ", "Engy" );
#                    if ( NCOMP_PASSIVE > 0 )
                     for (int v=0; v<NCOMP_PASSIVE; v++)
                     Aux_Message( stderr, "  %21s", PassiveFieldName_Grid[v] );
#                    endif
                     Aux_Message( stderr, "  %21s\n", "Pres" );

                     Pass = false;
                  }

                  Aux_Message( stderr, "%4d  %7d  (%5d,%5d,%5d)  (%2d,%2d,%2d)  %21.14e  %21.14e  %21.14e  %21.14e  %21.14e",
                               MPI_Rank, PID, amr->patch[0][lv][PID]->corner[0],
                                              amr->patch[0][lv][PID]->corner[1],
                                              amr->patch[0][lv][PID]->corner[2], i, j, k,
                               Fluid[DENS], Fluid[MOMX], Fluid[MOMY], Fluid[MOMZ], Fluid[ENGY] );
#                 if ( NCOMP_PASSIVE > 0 )
                  for (int v=NCOMP_FLUID; v<NCOMP_TOTAL; v++)
                  Aux_Message( stderr, "  %21.14e", Fluid[v] );
#                 endif
                  Aux_Message( stderr, "  %21.14e\n", Pres );
               }
            } // if ( Mode == 1  ||  Mode == 3 )

            if ( Mode == 2  ||  Mode == 3 )
            {
//             currently we use TINY_NUMBER as the floor value of entropy and hence here we use 2.0*TINY_NUMBER to validate entropy
#              if ( DUAL_ENERGY == DE_ENTROPY )
               if ( Pres <= (real)0.0  ||  Fluid[ENTROPY] < (real)2.0*TINY_NUMBER )
#              else
               if ( Pres <= (real)0.0 )
#              endif
               {
                  if ( Pass )
                  {
                     Aux_Message( stderr, "\"%s\" : <%s> FAILED at level %2d, Time = %13.7e, Step = %ld !!\n",
                                  comment, __FUNCTION__, lv, Time[lv], Step );
                     Aux_Message( stderr, "%4s  %7s  %19s  %10s  %21s  %21s  %21s  %21s  %21s",
                                  "Rank", "PID", "Patch Corner", "Grid ID", "Dens", "MomX", "MomY", "MomZ", "Engy" );
#                    if ( NCOMP_PASSIVE > 0 )
                     for (int v=0; v<NCOMP_PASSIVE; v++)
                     Aux_Message( stderr, "  %21s", PassiveFieldName_Grid[v] );
#                    endif
                     Aux_Message( stderr, "  %21s\n", "Pres" );

                     Pass = false;
                  }

                  Aux_Message( stderr, "%4d  %7d  (%5d,%5d,%5d)  (%2d,%2d,%2d)  %21.14e  %21.14e  %21.14e  %21.14e  %21.14e",
                               MPI_Rank, PID, amr->patch[0][lv][PID]->corner[0],
                                              amr->patch[0][lv][PID]->corner[1],
                                              amr->patch[0][lv][PID]->corner[2], i, j, k,
                               Fluid[DENS], Fluid[MOMX], Fluid[MOMY], Fluid[MOMZ], Fluid[ENGY] );
#                 if ( NCOMP_PASSIVE > 0 )
                  for (int v=NCOMP_FLUID; v<NCOMP_TOTAL; v++)
                  Aux_Message( stderr, "  %21.14e", Fluid[v] );
#                 endif
                  Aux_Message( stderr, "  %21.14e\n", Pres );
               }
            } // if ( Mode == 2  ||  Mode == 3 )

         } // i,j,k,PID
      } // if ( MPI_Rank == TargetRank )

      MPI_Bcast( &Pass, 1, MPI_INT, TargetRank, MPI_COMM_WORLD );

      MPI_Barrier( MPI_COMM_WORLD );

   } // for (int TargetRank = 0; TargetRank<MPI_NRank; TargetRank++)


   if ( Pass )
   {
      if ( MPI_Rank == 0 )
         Aux_Message( stdout, "\"%s\" : <%s> PASSED at level %2d, Time = %13.7e, Step = %ld\n",
                      comment, __FUNCTION__, lv, Time[lv], Step );
   }

} // FUNCTION : Hydro_Aux_Check_Negative



#endif // #if ( MODEL == HYDRO )
