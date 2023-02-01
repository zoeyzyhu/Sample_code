// Query 5
// Find the oldest friend for each user who has a friend. For simplicity,
// use only year of birth to determine age, if there is a tie, use the
// one with smallest user_id. You may find query 2 and query 3 helpful.
// You can create selections if you want. Do not modify users collection.
// Return a javascript object : key is the user_id and the value is the oldest_friend id.
// You should return something like this (order does not matter):
// {user1:userx1, user2:userx2, user3:userx3,...}

function oldest_friend(dbname){
  db = db.getSiblingDB(dbname);
  var results = {};

  db.createCollection("user_friends");
  db.users.aggregate([
    {$unwind: "$friends"},
    {$project: {
      _id:0,
      user_id: 1,
      friends: 1,
    }},
    {$out: "flat_users2"}
  ]);

  db.flat_users2.find().forEach(
    function(myDoc) {
      db.user_friends.insert( [
        {"user1":myDoc.user_id, "user2":myDoc.friends}, 
        {"user1":myDoc.friends, "user2":myDoc.user_id}
      ] );
    }
  );

  //put birth years into an array using user_id as the index
  var yob = {};
    db.users.find().forEach(function(myDoc2) {yob[myDoc2.user_id] = myDoc2.YOB;});

  
  db.user_friends.aggregate( [
    {
      $group: {
        _id: "$user1", 
        friend: {$push: "$user2"}}
    }
  ]).forEach(
    function(user){
      //init values for _id, year of birth of the first friend, and the first friend
      var uid = user._id;
      var maxYear = yob[user.friend[0]];
      var oldestFriend = user.friend[0];
      //loop through friends and check if friend[i]'s age is larger than the current friend
      //if it is larger, update the max friend. Older friend will have a smaller year
      for(i = 0; i < user.friend.length; i++){
        //if next friend is older than current friend, update the year and which friend is the oldest
        if(maxYear > yob[user.friend[i]]){
          maxYear = yob[user.friend[i]];
          oldestFriend = user.friend[i];
        }
        else if(maxYear == yob[user.friend[i]]){
          //if next friend is the same age, take lowest user_id
          oldestFriend = Math.min(oldestFriend, user.friend[i]);
        }
      }

      results[uid] = oldestFriend;
    });
  return results;
}
