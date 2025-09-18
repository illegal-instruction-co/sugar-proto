#include "user.sugar.h"

#include <iostream>
#include <string_view>

using namespace std;

void dump(int32_t id, string_view city) {
  cout << "DUMP:" << endl;
  cout << "id: " << id << endl;
  cout << "city: " << city << endl;
}

int main() {
  User userRaw;
  UserWrapped sugar(userRaw);

  // primitives
  sugar.id = 123;
  sugar.name = "berkay";
  sugar.active = true;
  sugar.score = 98.7;
  sugar.status = Status::OK;

  // repeated string
  sugar.tags.push_back("cpp");
  sugar.tags.push_back("protobuf");

  // repeated int
  sugar.numbers.push_back(10);
  sugar.numbers.push_back(20);

  // repeated nested
  sugar.profiles.push_back([](ProfileWrapped p) {
    p.city = "Istanbul";
    p.country = "TR";
  });

  sugar.profiles.push_back([](ProfileWrapped p) {
    p.city = "Berlin";
    p.country = "DE";
  });

  // map
  sugar.meta.set("lang", "c++");
  sugar.meta.set("level", "senior");

  // oneof
  sugar.contact.set("email", [](auto &msg, auto &field) {
    msg.GetReflection()->SetString(&msg, &field, "test@example.com");
  });

  // nested single
  sugar.profile.city = "London";
  sugar.profile.country = "UK";

  // raw proto
  cout << "RAW PROTOBUF:\n" << userRaw.DebugString() << endl;

  // sugar reads
  cout << "\nSUGAR READS:" << endl;
  cout << "id: " << sugar.id << endl;
  cout << "name: " << sugar.name << endl;
  cout << "active: " << sugar.active << endl;
  cout << "score: " << sugar.score << endl;
  cout << "status: " << sugar.status << endl;

  cout << "tags:" << endl;
  for (const auto &t : sugar.tags)
    cout << "  - " << t << endl;
  cout << "first tag: " << sugar.tags[0] << endl;

  cout << "numbers:" << endl;
  for (auto n : sugar.numbers)
    cout << "  - " << n << endl;

  cout << "profiles:" << endl;
  for (int i = 0; i < sugar.profiles.size(); i++) {
    ProfileWrapped p(sugar.profiles[i]);
    cout << "  profile[" << i << "]: " << p.city << ", " << p.country << endl;
  }

  cout << "meta fields set via map:" << endl;

  cout << "profile.city: " << sugar.profile.city << endl;
  cout << "profile.country: " << sugar.profile.country << endl;

  cout << "contact active field: " << sugar.contact.active_field()->name()
       << endl;

  dump(sugar.id, sugar.profile.city);

  return 0;
}
