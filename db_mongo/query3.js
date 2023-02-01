// Query 3
// Create a collection "cities" to store every user that lives in every city. Each document(city) has following schema:
// {
//   _id: city
//   users:[userids]
// }
// Return nothing.

function cities_table(dbname) {
    db = db.getSiblingDB(dbname);
    
    db.createCollection("cities");

    lcity = db.users.distinct("current.city");

    for (i=0; i < lcity.length; i++) {
        luser = db.users.find(
            {"current.city": lcity[i]},
            {user_id: 1}
        ).map(function(user) {
                  return user.user_id;
              });

        db.cities.insert(
            {"_id" : lcity[i], "users": luser}
        );
    }

    return;
}
