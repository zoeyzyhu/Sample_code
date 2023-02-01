// Query 6
// Find the average friend count per user.
// Return a decimal value as the average user friend count of all users in the users collection.

function find_average_friendcount(dbname){
    db = db.getSiblingDB(dbname)
    // TODO: return a decimal number of average friend count
    db.users.aggregate( [
        { $unwind : "$friends" },
        { $addFields: { userId : "$_id" }},
        { $project: { user_id : 1 , friends : 1, _id:0 }},
        { $out : "flat_users" },
    ] );

    return 1.0 * db.flat_users.find().count() / db.users.find().count();
}
