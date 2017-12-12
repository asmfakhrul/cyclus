
#include "cyclus.h"
#include <gtest/gtest.h>

using pyne::nucname::id;

namespace cyclus {

TEST(MockTests, Source) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  Composition::Ptr fresh = Composition::CreateFromMass(m);

  std::string config =
      "<commod>enriched_u</commod>"
      "<recipe_name>fresh_fuel</recipe_name>"
      "<capacity>10</capacity>";

  int dur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":agents:Source"), config, dur);
  sim.AddSink("enriched_u").Finalize();
  sim.AddRecipe("fresh_fuel", fresh);

  EXPECT_NO_THROW(sim.Run());

  SqliteBack b = sim.db();

  // Initial sources/sinks are instantiated before the agent being tested.
  // Check that Source comes last in the table.
  QueryResult qr = b.Query("AgentEntry", NULL);
  EXPECT_EQ(2, qr.rows.size()) << "expected 2 agents, got " << qr.rows.size();
  EXPECT_EQ(":agents:Source", qr.GetVal<std::string>("Spec", 1));
  EXPECT_EQ(":agents:Sink", qr.GetVal<std::string>("Spec", 0));

  qr = b.Query("Transactions", NULL);
  EXPECT_EQ(dur, qr.rows.size()) << "expected " << dur << " transactions, got "
                                 << qr.rows.size();
  for (int i = 0; i < qr.rows.size(); i++) {
    int transaction_t = qr.GetVal<int>("Time", i);
    EXPECT_EQ(i, transaction_t);
  }
}

TEST(MockTests, Sink) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  Composition::Ptr fresh = Composition::CreateFromMass(m);

  std::string config =
      "<in_commods><val>enriched_u</val></in_commods>"
      "<recipe_name>fresh_fuel</recipe_name>"
      "<capacity>10</capacity>";

  int dur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":agents:Sink"), config, dur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddRecipe("fresh_fuel", fresh);

  EXPECT_NO_THROW(sim.Run());
  SqliteBack b = sim.db();

  // Initial sources/sinks are instantiated before the agent being tested.
  // Check that Sink comes last in the table.
  QueryResult qr = b.Query("AgentEntry", NULL);
  EXPECT_EQ(2, qr.rows.size()) << "expected 2 agents, got " << qr.rows.size();
  EXPECT_EQ(":agents:Sink", qr.GetVal<std::string>("Spec", 1));
  EXPECT_EQ(":agents:Source", qr.GetVal<std::string>("Spec", 0));

  qr = b.Query("Transactions", NULL);
  EXPECT_EQ(dur, qr.rows.size()) << "expected " << dur << " transactions, got "
                                 << qr.rows.size();
  for (int i = 0; i < qr.rows.size(); i++) {
    int transaction_t = qr.GetVal<int>("Time", i);
    EXPECT_EQ(i, transaction_t);
  }
}

TEST(MockTests, ReconstructMaterial) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  compmath::Normalize(&m);
  Composition::Ptr fresh = Composition::CreateFromMass(m);

  std::string config =
      "<in_commods><val>enriched_u</val></in_commods>"
      "<recipe_name>fresh_fuel</recipe_name>"
      "<capacity>10</capacity>";

  int dur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":agents:Sink"), config, dur);
  sim.AddSource("enriched_u").Finalize();
  sim.AddRecipe("fresh_fuel", fresh);

  sim.Run();

  QueryResult qr = sim.db().Query("Transactions", NULL);
  Material::Ptr mat = sim.GetMaterial(qr.GetVal<int>("ResourceId"));
  toolkit::MatQuery mq(mat);

  double cap = 10;
  cyclus::CompMap::iterator it;
  for (it = m.begin(); it != m.end(); ++it) {
    cyclus::Nuc nuc = it->first;
    EXPECT_DOUBLE_EQ(m[nuc]*cap, mq.mass(nuc));
  }
}

// mock sim should not build the agent being tested until Run is called - this
// is important because recipes that it may access cannot be added until after
// mock sim is constructed.
TEST(MockTests, BuildAfterConstruct) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  Composition::Ptr fresh = Composition::CreateFromMass(m);

  std::string config =
      "<in_commods><val>enriched_u</val></in_commods>"
      "<recipe_name>fresh_fuel</recipe_name>"
      "<capacity>10</capacity>";

  int dur = 10;
  cyclus::MockSim sim(cyclus::AgentSpec(":agents:Sink"), config, dur);
  sim.AddRecipe("fresh_fuel", fresh);

  EXPECT_EQ(-1, sim.agent->enter_time());
  sim.Run();
  EXPECT_EQ(0, sim.agent->enter_time());
}

// mock sim should decommission an agent being tested if a lifetime is given.
TEST(MockTests, FiniteLifeDecommission) {
  cyclus::CompMap m;
  m[id("U235")] = .05;
  m[id("U238")] = .95;
  Composition::Ptr fresh = Composition::CreateFromMass(m);

  std::string config =
      "<in_commods><val>enriched_u</val></in_commods>"
      "<recipe_name>fresh_fuel</recipe_name>"
      "<capacity>10</capacity>";

  int life = 5;
  int dur = 2 * life;
  cyclus::MockSim sim(cyclus::AgentSpec(":agents:Sink"), config, dur, life);
  sim.AddRecipe("fresh_fuel", fresh);
  int agentid = sim.Run();
  cyclus::SqlStatement::Ptr stmt = sim.db().db().Prepare(
      "SELECT exittime FROM agentexit WHERE agentid=?;"
      );

  stmt->BindInt(1, agentid);
  stmt->Step();
  EXPECT_EQ(life - 1, stmt->GetInt(0));
}

}  // namespace cyclus
