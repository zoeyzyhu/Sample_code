// Query 4
// Find user pairs (A,B) that meet the following constraints:
// i) user A is male and user B is female
// ii) their Year_Of_Birth difference is less than year_diff
// iii) user A and B are not friends
// iv) user A and B are from the same hometown city
// The following is the schema for output pairs:
// [
//      [user_id1, user_id2],
//      [user_id1, user_id3],
//      [user_id4, user_id2],
//      ...
//  ]
// user_id is the field from the users collection. Do not use the _id field in users.
// Return an array of arrays.

function suggest_friends(year_diff, dbname) {
    db = db.getSiblingDB(dbname);

    var pairs = [];
    // TODO: implement suggest friends

    // find male-female user pairs who are from the same hometown and born in years less different than year_diff
    db.users.find().forEach(function(doc) {
        if (doc.gender == "male") {
            db.users.find().forEach (function(doc2) {
                if (doc2.gender == "female") {
                    if (doc.hometown.city == doc2.hometown.city && // same hometown
                        Math.abs(doc2.YOB - doc.YOB) < year_diff &&  // YOB diff < year_diff
                        doc2.friends.indexOf(doc.user_id) == -1 && // male is not in the female's friends
                        doc.friends.indexOf(doc2.user_id) == -1 // female is not in the male's friends
                    ) {
                        const pair = [doc.user_id, doc2.user_id];
                        pairs.push(pair)
                    }
                }
            })
        }
    })

    return pairs;
}
