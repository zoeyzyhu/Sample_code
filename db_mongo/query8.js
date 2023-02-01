// Query 8
// Find the city average friend count per user using MapReduce.

var city_average_friendcount_mapper = function () {
    // TODO: Implement the map function
    emit(this.hometown.city, {user_count: 1, friends: this.friends.length});
};

var city_average_friendcount_reducer = function (key, values) {
    // TODO: Implement the reduce function
    var reduceVal = {user_count: 0, friends: 0};
    for (var i = 0; i < values.length; i++) {
        reduceVal.user_count += values[i].user_count;
        reduceVal.friends += values[i].friends;
    }
    return reduceVal;
};

var city_average_friendcount_finalizer = function (key, reduceVal) {
    // We've implemented a simple forwarding finalize function. This implementation
    // is naive: it just forwards the reduceVal to the output collection.
    // TODO: Feel free to change it if needed.
    return 1.0*reduceVal.friends/reduceVal.user_count;
};
