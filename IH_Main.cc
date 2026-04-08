// File IH_Main.cc
#include "IH_Helpers.hh"

using namespace EasyLocal::Debug;
int main(int argc, const char *argv[])
{
  ParameterBox main_parameters("main", "Main Program options");
  Parameter<string> instance("instance", "Input instance", main_parameters); 
  Parameter<unsigned> seed("seed", "Random seed", main_parameters);
  Parameter<string> method("method", "Solution method (empty for tester)", main_parameters);   
  Parameter<string> init_state("init_state", "Initial state (to be read from file)", main_parameters);
  Parameter<string> output_file("output_file", "Write the output to a file (filename required)", main_parameters);
  Parameter<double> change_room_rate("change_room_rate","Probability of select a change room move", main_parameters); 
  Parameter<double> change_ot_rate("change_ot_rate","Probability of select a change ot move", main_parameters);
  Parameter<double> change_nurse_rate("change_nurse_rate","Probability of select a change nurse move", main_parameters);
  Parameter<double> swap_patients_rate("swap_patients","Probability of select a swap patients move", main_parameters);   
  change_nurse_rate=0.167;
  change_ot_rate=0.141;
  change_room_rate=0.096;
  swap_patients_rate=0.289;
  CommandLineParameters::Parse(argc, argv, false, true); 
  if (!instance.IsSet())
    {
      throw invalid_argument("Instance file name must be provided. Use --instance <file_path> to specify it.");
    }
  IH_Input in(instance);
  if (seed.IsSet())
    Random::SetSeed(seed);

  IH_SolutionManager sm(in);
  IH_MoveAdmissionNeighborhoodExplorer mahe(in, sm);
  IH_ChangeOTNeighborhoodExplorer cthe(in, sm);
  IH_ChangeRoomNeighborhoodExplorer crhe(in, sm);
  IH_ChangeNurseNeighborhoodExplorer cnhe(in, sm);
  IH_SwapPatientsNeighborhoodExplorer sphe(in, sm);

  IH_OvertimeSurgeon cc1 (in, 1, true);
  IH_OvertimeOT cc2 (in, 1, true);
  IH_OverCapacity cc3(in, 1, true);
  IH_GenderMix cc4 (in, 1, true);
  IH_Unscheduled cc5 (in, in.Weight(0), false);
  IH_Delay cc6 (in, in.Weight(1), false); 
  IH_OpenOT cc7(in,in.Weight(2), false);
  IH_AgeMix cc8 (in, in.Weight(3), false);
  IH_Skill cc9 (in, in.Weight(4), false);
  IH_Excess cc10 (in,in.Weight(5), false);
  IH_Continuity cc11 (in, in.Weight(6), false);
  IH_SurgeonTransfer cc12 (in, in.Weight(7), false);

  IH_MoveAdmissionDeltaOvertimeSurgeon dcc1(in,cc1);
  IH_MoveAdmissionDeltaOvertimeOT dcc2(in,cc2);
  IH_MoveAdmissionDeltaOverCapacity dcc3(in,cc3);
  IH_MoveAdmissionDeltaGenderMix dcc4(in,cc4);
  IH_MoveAdmissionDeltaUnscheduled dcc5(in,cc5);
  IH_MoveAdmissionDeltaDelay dcc6(in,cc6);
  IH_MoveAdmissionDeltaOpenOT dcc7(in,cc7);
  IH_MoveAdmissionDeltaAgeMix dcc8(in,cc8);
  IH_MoveAdmissionDeltaSkill dcc9(in,cc9);
  IH_MoveAdmissionDeltaExcess dcc10(in,cc10);
  IH_MoveAdmissionDeltaContinuity dcc11(in,cc11);
  IH_MoveAdmissionDeltaSurgeonTransfer dcc12(in,cc12);
  
  IH_ChangeOTDeltaOvertimeOT dctcc2(in,cc2);
  IH_ChangeOTDeltaOpenOT dctcc7(in,cc7);
  IH_ChangeOTDeltaSurgeonTransfer dctcc12(in,cc12);

  IH_ChangeRoomDeltaOverCapacity dcrcc3(in,cc3);
  IH_ChangeRoomDeltaGenderMix dcrcc4(in,cc4);
  IH_ChangeRoomDeltaAgeMix dcrcc8(in,cc8);
  IH_ChangeRoomDeltaSkill dcrcc9(in,cc9);
  IH_ChangeRoomDeltaExcess dcrcc10(in,cc10);
  IH_ChangeRoomDeltaContinuity dcrcc11(in,cc11);

  IH_ChangeNurseDeltaSkill dcncc9(in,cc9);
  IH_ChangeNurseDeltaExcess dcncc10(in,cc10);
  IH_ChangeNurseDeltaContinuity dcncc11(in,cc11);

  IH_SwapPatientsDeltaOvertimeSurgeon dspcc1(in,cc1);
  IH_SwapPatientsDeltaOvertimeOT dspcc2(in,cc2);
  IH_SwapPatientsDeltaOverCapacity dspcc3(in,cc3);
  IH_SwapPatientsDeltaGenderMix dspcc4(in,cc4);
  IH_SwapPatientsDeltaDelay dspcc6(in,cc6);  
  IH_SwapPatientsDeltaAgeMix dspcc8(in,cc8);
  IH_SwapPatientsDeltaSkill dspcc9(in,cc9);
  IH_SwapPatientsDeltaExcess dspcc10(in,cc10);
  IH_SwapPatientsDeltaContinuity dspcc11(in,cc11);
  IH_SwapPatientsDeltaSurgeonTransfer dspcc12(in,cc12);

  sm.AddCostComponent(cc1);
  sm.AddCostComponent(cc2);
  sm.AddCostComponent(cc3);
  sm.AddCostComponent(cc4);
  sm.AddCostComponent(cc5);
  sm.AddCostComponent(cc6);
  sm.AddCostComponent(cc7);
  sm.AddCostComponent(cc8);
  sm.AddCostComponent(cc9);
  sm.AddCostComponent(cc10);
  sm.AddCostComponent(cc11);
  sm.AddCostComponent(cc12);

  mahe.AddDeltaCostComponent(dcc1);
  mahe.AddDeltaCostComponent(dcc2);
  mahe.AddDeltaCostComponent(dcc3);
  mahe.AddDeltaCostComponent(dcc4);
  mahe.AddDeltaCostComponent(dcc5);
  mahe.AddDeltaCostComponent(dcc6);
  mahe.AddDeltaCostComponent(dcc7);
  mahe.AddDeltaCostComponent(dcc8);
  mahe.AddDeltaCostComponent(dcc9);
  mahe.AddDeltaCostComponent(dcc10);
  mahe.AddDeltaCostComponent(dcc11);
  mahe.AddDeltaCostComponent(dcc12);
  
  cthe.AddDeltaCostComponent(dctcc2);
  cthe.AddDeltaCostComponent(dctcc7);
  cthe.AddDeltaCostComponent(dctcc12);

  crhe.AddDeltaCostComponent(dcrcc3);
  crhe.AddDeltaCostComponent(dcrcc4);
  crhe.AddDeltaCostComponent(dcrcc8);
  crhe.AddDeltaCostComponent(dcrcc9);
  crhe.AddDeltaCostComponent(dcrcc10);
  crhe.AddDeltaCostComponent(dcrcc11);

  cnhe.AddDeltaCostComponent(dcncc9);
  cnhe.AddDeltaCostComponent(dcncc10);
  cnhe.AddDeltaCostComponent(dcncc11);

  sphe.AddDeltaCostComponent(dspcc1);
  sphe.AddDeltaCostComponent(dspcc2);
  sphe.AddDeltaCostComponent(dspcc3);
  sphe.AddDeltaCostComponent(dspcc4);
  sphe.AddDeltaCostComponent(dspcc6);
  sphe.AddDeltaCostComponent(dspcc8);
  sphe.AddDeltaCostComponent(dspcc9);
  sphe.AddDeltaCostComponent(dspcc10);
  sphe.AddDeltaCostComponent(dspcc11);
  sphe.AddDeltaCostComponent(dspcc12);

  SetUnionNeighborhoodExplorer<IH_Input, IH_Output, DefaultCostStructure<int>,decltype(mahe),decltype(cthe),decltype(crhe),decltype(cnhe)> 
  qmhe(in, sm, "Quadmodal MA,COT,CR,CN", mahe,cthe,crhe,cnhe,{1.0 - (change_ot_rate+change_nurse_rate+change_room_rate),change_ot_rate,change_room_rate,change_nurse_rate});

  SetUnionNeighborhoodExplorer<IH_Input, IH_Output, DefaultCostStructure<int>,decltype(mahe),decltype(cthe),decltype(crhe),decltype(cnhe),decltype(sphe)> 
  pmhe(in, sm, "Pentamodal MA,COT,CR,CN,SP", mahe,cthe,crhe,cnhe,sphe,{1.0 - (change_ot_rate+change_nurse_rate+change_room_rate+swap_patients_rate),change_ot_rate,change_room_rate,change_nurse_rate,swap_patients_rate});

  HillClimbing<IH_Input, IH_Output, IH_MoveAdmission, DefaultCostStructure<int>> mhc(in, sm, mahe, "MHC");
  SteepestDescent<IH_Input, IH_Output, IH_MoveAdmission, DefaultCostStructure<int>> msd(in, sm, mahe, "MSD");
  SimulatedAnnealing<IH_Input, IH_Output, IH_MoveAdmission, DefaultCostStructure<int>> msa(in, sm, mahe, "IH_MSA");
  SimulatedAnnealing<IH_Input, IH_Output,decltype(qmhe)::MoveType,DefaultCostStructure<int>> qsa(in, sm, qmhe, "IH_QSA");
  SimulatedAnnealing<IH_Input, IH_Output,decltype(pmhe)::MoveType,DefaultCostStructure<int>> psa(in, sm, pmhe, "IH_PSA");
  SimulatedAnnealingTimeBased<IH_Input, IH_Output,decltype(pmhe)::MoveType,DefaultCostStructure<int>> psa_tb(in, sm, pmhe, "IH_TB_PSA");
  
  Tester<IH_Input, IH_Output> tester(in, sm);
  MoveTester<IH_Input, IH_Output, IH_MoveAdmission> move_ad_test(in, sm, mahe, "IH_MoveAdmission", tester); 
  MoveTester<IH_Input, IH_Output, IH_ChangeOT> change_ot_test(in, sm, cthe, "IH_ChangeOT", tester); 
  MoveTester<IH_Input, IH_Output, IH_ChangeRoom> change_room_test(in, sm, crhe, "IH_ChangeRoom", tester); 
  MoveTester<IH_Input, IH_Output, IH_ChangeNurse> change_nurse_test(in, sm, cnhe, "IH_ChangeNurse", tester); 
  MoveTester<IH_Input, IH_Output,IH_SwapPatients> swap_patients_test(in, sm, sphe, "IH_SwapPatients", tester);
  MoveTester<IH_Input, IH_Output,decltype(qmhe)::MoveType,DefaultCostStructure<int>> qsa_test(in, sm, qmhe, "IH_Quadmodal", tester); 

  SimpleLocalSearch<IH_Input, IH_Output, DefaultCostStructure<int>> solver(in, sm, "IH solver");

  if (!CommandLineParameters::Parse(argc, argv, true, false))
    return 1;
  if (!method.IsSet())
    {
      if (init_state.IsSet())
        tester.RunMainMenu(init_state);
      else
        tester.RunMainMenu();   
    }
  else{
    if (method==string("MHC"))
    {
      solver.SetRunner(mhc);
    }
    else if (method == string("MSD"))
    {
      solver.SetRunner(msd);
    }
    else if (method == string("MSA"))
    {
      solver.SetRunner(msa);
    }
    else if (method == string("QSA"))
    {
      solver.SetRunner(qsa);
    }
    else if (method == string("PSA"))
    {
      solver.SetRunner(psa);
    }
    else if (method == string("PSA_TB"))
    {
      solver.SetRunner(psa_tb);
    }
    else
    {
      throw invalid_argument("Unknown method" + static_cast<string>(method) + " check valid options");
    } 
    auto result = solver.Solve();
    IH_Output out = result.output;
    cout  << "{" 
      << "\"violations\": " <<  result.cost.violations <<  ", "
      << "\"cost\": " <<  result.cost.total <<  ", "
      << "\"time\": " <<  result.running_time <<  ", "
      << "\"seed\": " << Random::GetSeed()<<"}" << endl;
    if (output_file.IsSet())
    { 
      ofstream os(static_cast<string>(output_file).c_str());
      string info="Cost: "+ to_string(result.cost.total);
      for (size_t i=4; i<result.cost.all_components.size(); i++)
        info+=", "+sm.GetCostComponent(i).name+ ":"+to_string(result.cost.all_components[i]);
      out.PrettyPrint(os,info);
    }
  }
  return 0;
}
