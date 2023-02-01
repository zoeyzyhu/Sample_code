import java.io.FileWriter;
import java.io.IOException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.TreeSet;
import java.util.Vector;

import org.json.JSONObject;
import org.json.JSONArray;

public class GetData {

    static String prefix = "project3.";

    // You must use the following variable as the JDBC connection
    Connection oracleConnection = null;

    // You must refer to the following variables for the corresponding 
    // tables in your database
    String userTableName = null;
    String friendsTableName = null;
    String cityTableName = null;
    String currentCityTableName = null;
    String hometownCityTableName = null;

    // DO NOT modify this constructor
    public GetData(String u, Connection c) {
        super();
        String dataType = u;
        oracleConnection = c;
        userTableName = prefix + dataType + "_USERS";
        friendsTableName = prefix + dataType + "_FRIENDS";
        cityTableName = prefix + dataType + "_CITIES";
        currentCityTableName = prefix + dataType + "_USER_CURRENT_CITIES";
        hometownCityTableName = prefix + dataType + "_USER_HOMETOWN_CITIES";
    }

    // TODO: Implement this function
    @SuppressWarnings("unchecked")
    public JSONArray toJSON() throws SQLException {

        // This is the data structure to store all users' information
        JSONArray users_info = new JSONArray();
        
        try (Statement stmt = oracleConnection.createStatement(ResultSet.TYPE_SCROLL_INSENSITIVE, ResultSet.CONCUR_READ_ONLY)) {
            // Your implementation goes here....

            // get users and their basic info
            ResultSet rst = stmt.executeQuery("SELECT * FROM " + userTableName + "");

            Vector<JSONObject> users = new Vector<JSONObject>();

            while(rst.next()) {
                JSONObject user = new JSONObject();
                user.put("user_id", rst.getInt(1));
                user.put("first_name", rst.getString(2));
                user.put("last_name", rst.getString(3));
                user.put("YOB", rst.getInt(4));
                user.put("MOB", rst.getInt(5));
                user.put("DOB", rst.getInt(6));
                user.put("gender", rst.getString(7));

                users.add(user);
            }

            // get users' friends, hometown locations, and current locations
            for (int i = 0; i < users.size(); i++) {
                JSONObject u = users.get(i);
                int uid = u.getInt("user_id");

                // query friends of uid w ids greater than uid
                rst = stmt.executeQuery(
                    "SELECT USER2_ID " +
                    "FROM " + friendsTableName + " " +
                    "WHERE ( " +
                    "    USER1_ID = " + uid + " AND " +
                    "    USER2_ID > USER1_ID " +
                    ")"
                );
                JSONArray friends = new JSONArray();
                while (rst.next()) {
                    friends.put(rst.getInt(1));
                }
                u.put("friends", friends);

                //get users hometown city, state, country
                rst = stmt.executeQuery(
                    "SELECT C.City_Name, C.State_Name, C.Country_Name " +
                    "FROM " + hometownCityTableName + " HC, " + cityTableName + " C " +
                    "WHERE ( " +
                    "    HC.User_ID = " + uid + " AND " +
                    "    C.City_ID = HC.Hometown_City_ID " +
                    ")"
                );
                JSONObject hometown = new JSONObject();
                while (rst.next()) {
                    hometown.put("city", rst.getString(1));
                    hometown.put("state", rst.getString(2));
                    hometown.put("country", rst.getString(3));
                }
                u.put("hometown", hometown);

                //get users current city, state, country
                rst = stmt.executeQuery(
                    "SELECT C.City_Name, C.State_Name, C.Country_Name " +
                    "FROM " + currentCityTableName + " CC, " + cityTableName + " C " +
                    "WHERE ( " +
                    "    CC.User_ID = " + uid + " AND " +
                    "    C.City_ID = CC.Current_City_ID " +
                    ")"
                );
                JSONObject current = new JSONObject();
                while (rst.next()) {
                    current.put("city", rst.getString(1));
                    current.put("state", rst.getString(2));
                    current.put("country", rst.getString(3));
                }
                u.put("current", current);


                // add user to to result JSONArray
                users_info.put(u);
            }
            
            rst.close();
            stmt.close();
        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return users_info;
    }

    // This outputs to a file "output.json"
    // DO NOT MODIFY this function
    public void writeJSON(JSONArray users_info) {
        try {
            FileWriter file = new FileWriter(System.getProperty("user.dir") + "/output.json");
            file.write(users_info.toString());
            file.flush();
            file.close();

        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
