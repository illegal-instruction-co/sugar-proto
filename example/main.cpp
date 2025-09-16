#include "user.pb.h"

#include "user.sugar.h"

#include <iostream>

using namespace std;

int main() {
  User userRaw;
  UserWrapped sugar(userRaw);

  sugar.id = 123;
  sugar.tags.push_back("cpp");
  sugar.profile.city = "ISTANBUL";

  cout << userRaw.DebugString() << endl;

  return 0;
}
