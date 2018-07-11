// This tool makes the ROOT trees needed to reproduce the plots from the ANNIE
// Phase I publication about beam-induced neutron backgrounds in SciBooNE hall.
//
// Steven Gardiner <sjgardiner@ucdavis.edu>
#pragma once

// standard library includes
#include <map>
#include <memory>
#include <string>
#include <vector>

// ROOT includes
#include "TFile.h"
#include "TTree.h"

// ToolAnalysis includes
#include "Tool.h"

struct NCVPositionInfo {
  NCVPositionInfo(int r = 0, int s = 0) : run(r), subrun(s) {}
  double total_POT = 0.;
  uint64_t num_beam_spills = 0ull;
  uint64_t num_source_triggers = 0ull;
  uint64_t num_cosmic_triggers = 0ull;
  uint64_t num_soft_triggers = 0ull;
  uint64_t num_led_triggers = 0ull;
  int run = 0;
  int subrun = 0;
};

class ADCPulse;
class ChannelKey;

class PhaseITreeMaker : public Tool {

  public:

    PhaseITreeMaker();
    bool Initialise(const std::string configfile, DataModel& data) override;
    bool Execute() override;
    bool Finalise() override;

  protected:

    template <typename T, typename AStore> bool get_object_from_store(
      const std::string& object_label, T& obj, AStore& s)
    {
      Log("Retrieving \"" + object_label + "\" from a Store", 4, verbosity_);
      bool got_object = s.Get(object_label, obj);

      // Check for problems
      if ( !got_object ) {
        Log("Error: The PhaseITreeMaker tool could not find the " + object_label
          + " entry", 0, verbosity_);
        return false;
      }

      return true;
    }

    template <typename T> inline auto check_that_not_empty(
      const std::string& object_label, T& obj)
      -> decltype( std::declval<T&>().empty() )
    {
      bool is_empty = obj.empty();
      if ( is_empty ) {
        Log("Error: The PhaseITreeMaker tool found an empty " + object_label
          + " entry", 0, verbosity_);
      }

      return !is_empty;
    }

    int get_NCV_position(uint32_t run_number) const;

    bool approve_event(int64_t event_time, int64_t old_time,
      const ADCPulse& first_ncv1_pulse, const std::map<ChannelKey, std::vector<
      std::vector<ADCPulse> > >& adc_hits, int minibuffer_index, int pmt_id,
      const ADCPulse*& matching_pulse);

    double compute_tank_charge(size_t minibuffer_number,
      const std::map< ChannelKey, std::vector<
        std::vector<ADCPulse> > >& adc_hits, uint64_t start_time,
        uint64_t end_time, int& num_unique_water_pmts);

    void find_ncv_events(const std::vector< std::vector<ADCPulse> >& pulses,
      int pmt_id, int64_t& old_time, const std::map<ChannelKey, std::vector<
      std::vector<ADCPulse> > >& adc_hits, const HeftyInfo& hefty_info,
      const MinibufferLabel& event_mb_label, int mb);

    /// @brief Integer that determines the level of logging to perform
    int verbosity_ = 0;

    /// @brief The time (in ns) to use when applying the afterpulsing veto
    int afterpulsing_veto_time_ = 0;

    /// @brief The time interval over which to compute the tank charge for
    /// each NCV coincidence event
    int tank_charge_window_length_ = 0;

    /// @brief The maximum number of unique water PMTs to allow for a neutron
    /// candidate event
    int max_unique_water_pmts_ = 0;

    /// @brief The maximum tank charge (in nC) to allow for a neutron candidate
    /// event
    double max_tank_charge_ = 0.;

    /// @brief The maximum allowed time between NCV PMT pulses for them to
    /// count as a "coincidence"
    int ncv_coincidence_tolerance_ = 0;

    /// @brief ROOT TFile that will be used to store the output from this tool
    std::unique_ptr<TFile> output_tfile_ = nullptr;

    /// @brief TTree that will be used to store output
    TTree* output_tree_ = nullptr;

    // Branch variables
    uint32_t run_number_ = 0u;
    uint32_t subrun_number_ = 0u;
    uint32_t event_number_ = 0u;
    int ncv_position_ = 0;
    int64_t event_time_ns_ = 0; // ns
    uint8_t event_label_ = 0u;
    bool hefty_mode_ = false;
    int hefty_trigger_mask_ = 0;

    double amplitude_ncv1_ = 0.; // V
    double amplitude_ncv2_ = 0.; // V
    double charge_ncv1_ = 0.; // nC
    double charge_ncv2_ = 0.; // nC
    unsigned short raw_amplitude_ncv1_ = 0u; // ADC counts
    unsigned short raw_amplitude_ncv2_ = 0u; // ADC counts

    bool ncv1_fired_ = false;
    bool ncv2_fired_ = false;
    int64_t ncv1_pulse_time_ns_ = 0; // ns
    int64_t ncv2_pulse_time_ns_ = 0; // ns

    double tank_charge_; // nC
    int unique_hit_water_pmts_;
    int64_t time_since_last_event_; // ns

    // Cut pass flags
    bool passed_afterpulse_cut_ = false;
    bool passed_unique_water_pmt_cut_ = false;
    bool passed_tank_charge_cut_ = false;

    // Stores total POT, spill, etc. information for each NCV position
    std::map<int, NCVPositionInfo> ncv_position_info_;

    // Extra tree that stores information about each pulse found by the
    // ADCHitFinder tool, regardless of whether it is part of an NCV
    // coincidence event or not.
    TTree* output_pulse_tree_ = nullptr;

    // Pulse tree branch variables
    uint32_t minibuffer_number_ = 0u;
    int64_t pulse_start_time_ns_ = 0; // ns
    double pulse_amplitude_ = 0.; // V
    double pulse_charge_ = 0.; // nC
    int pulse_pmt_id_ = 0.; // nC
    unsigned short pulse_raw_amplitude_ = 0u; // ADC counts
    uint32_t spill_number_ = 0u;
    bool in_spill_ = false;

    // Tree that stores information about the beam quality cuts
    TTree* output_beam_tree_ = nullptr;

    // Beam tree branch variables
    bool pot_ok_ = false;
    bool horn_current_ok_ = false;
    bool timestamps_ok_ = false;
    bool toroids_agree_ = false;
};
