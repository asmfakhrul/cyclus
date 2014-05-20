#ifndef CYCLUS_SRC_COMMODITY_PRODUCER_MANAGER_H_
#define CYCLUS_SRC_COMMODITY_PRODUCER_MANAGER_H_

#include <set>

#include "agent.h"
#include "commodity.h"
#include "commodity_producer.h"

namespace cyclus {
namespace toolkit {

/// a mixin to provide information about commodity producers
class CommodityProducerManager {
 public: 
  CommodityProducerManager(Agent* agent=NULL) : agent_(agent) {};
  virtual ~CommodityProducerManager() {};

  /// @return the total production capacity for a commodity amongst producers
  /// @param commodity the commodity in question
  double TotalCapacity(Commodity& commodity);

  /// register a commodity producer with the manager
  /// @param producer the producer
  inline void Register(CommodityProducer* producer) {
    producers_.insert(producer);
  }

  /// unregister a commodity producer with the manager
  /// @param producer the producer
  inline void Unregister(CommodityProducer* producer) {
    producers_.erase(producer);
  }

  inline const std::set<CommodityProducer*>& producers() const {return producers_;}

  inline Agent* agent() const {return agent_;}

 private:
  /// the agent managing this instance
  Agent* agent_;

  /// the set of managed producers
  std::set<CommodityProducer*> producers_;
};

} // namespace toolkit
} // namespace cyclus
#endif  // CYCLUS_SRC_COMMODITY_PRODUCER_MANAGER_H_
