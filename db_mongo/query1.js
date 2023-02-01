// Query 1
// Find users who live in city "city".
// Return an array of user_ids. The order does not matter.

function find_user(city, dbname) {
    db = db.getSiblingDB(dbname);

    var results = [];

    db.users.find().forEach(
        function(doc) {
            if (doc.hometown.city == city) {
                results.push(doc.user_id);
            }
        }
    );

    return results;
}
