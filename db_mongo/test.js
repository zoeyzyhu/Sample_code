// fill in your database name
// Your dbname is your uniqname
var dbname = "hzin";

var l1 = load("query1.js");
var l2 = load("query2.js");
var l3 = load("query3.js");
var l4 = load("query4.js");
var l5 = load("query5.js");
var l6 = load("query6.js");
var l7 = load("query7.js");
var l8 = load("query8.js");

print("===Test1===");
var test1 = find_user("Bucklebury", dbname);
print(test1); // uncomment this line to print the query1 output
var ans1 = test1.length;
if (ans1 == 42) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected 42 users from Bucklebury, you found", ans1, "users.");
}

print("===Test2===");
unwind_friends(dbname);
var test2 = db.flat_users.find();
test2; // uncomment this line to print the query2 output
var ans2 = test2.count();
if (ans2 == 21355) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed! ");
    print("Expected 21355 pairs of friends, you found", ans2, "pairs.");
}

print("===Test3===");
cities_table(dbname);
var test3 = db.cities.find({ _id: "Bucklebury" });
var ans3 = 0;
if (test3.hasNext()) {
    var test3 = test3.next().users;
    print(test3) // uncomment this line to print the query3 output
    var ans3 = test3.length;
}
if (ans3 == 43) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected 43 users living in Bucklebury, you found", ans3, "users.");
}

print("===Test4===");
var test4 = suggest_friends(5, dbname);
// printjson(test4); // uncomment this line to print the query4 output
var ans4 = test4.length;
if (ans4 == 87) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected 87 pairs of suggested friends, you found", ans4, "pairs.");
}

print("===Test5=== (This test is time comsuming)");
var test5 = oldest_friend(dbname);
test5; // uncomment this line to print the query5 output
if (Object.keys(test5).length == 798) {
    if (test5.hasOwnProperty(799)) {
        var ans5 = test5[799];
        if (test5[799] == 51) {
            print("Local test passed! Partially correct.");
        } else {
            print("Local test failed!");
            print("Expected oldest friend for user 799 to be 51, you found", ans5, ".");
        }
    } else {
        print("Local test failed!");
        print("Expected oldest friend for user 799 to be 51. You found no oldest friend for user 799.");
    }
} else {
    print("Local test failed!");
    print("Expected 798 oldest friends for user 799, you found", Object.keys(test5).length, "friends.");
}

print("===Test6===");
var ans6 = find_average_friendcount(dbname);
// print(ans6); // uncomment this line to print the query6 output
if ((ans6 > 26) & (ans6 < 27)) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected the average number of friends to be between 26 to 27, you calculated", ans6, "friends.");
}

print("===Test7===");
var result7 = db.users.mapReduce(num_month_mapper, num_month_reducer, {
    out: "born_each_month",
    finalize: num_month_finalizer,
});
var test7 = db.born_each_month.find();
test7; // uncomment this line to print the query7 output
var ans7 = test7.count();
if (ans7 == 12) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected 12 months(keys), you found", ans7, "months.");
}

print("===Test8===");
var result8 = db.users.mapReduce(city_average_friendcount_mapper, city_average_friendcount_reducer, {
    out: "friend_city_population",
    finalize: city_average_friendcount_finalizer,
});
var test8 = db.friend_city_population.find();
test8; // unconmment this line to print the query8 output
var ans8 = test8.count();
if (ans8 == 16) {
    print("Local test passed! Partially correct.");
} else {
    print("Local test failed!");
    print("Expected 16 cities(keys), you found", ans8, "cities.");
}
