#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ORDERS 300
#define MAX_ITEMS 10
#define DATA_FILE "orders.dat"


/* =========================================================
                    COMMON ORDER STRUCTURE
   ========================================================= */

struct Order
{
    unsigned int order_id;
    char customer[50];

    unsigned int item_ids[10];
    unsigned char qty[10];

    unsigned char item_count;

    char status[12];
};

struct Order orders[MAX_ORDERS];

unsigned int orderCount = 0;


/* =========================================================
                    INPUT HELPER FUNCTIONS
   ========================================================= */

/*
   These functions prevent scanf() input problems.
   They also prevent buffer overflow.
*/

void read_line(char *str, int size)
{
    if (fgets(str, size, stdin) != NULL)
    {
        str[strcspn(str, "\n")] = '\0';
    }
}


/* Read unsigned int safely */
unsigned int read_unsigned_int()
{
    char input[100];
    char extra;
    unsigned int value;

    while (1)
    {
        read_line(input, sizeof(input));

        if (sscanf(input, "%u %c", &value, &extra) == 1)
        {
            return value;
        }

        printf("Invalid input. Enter a number: ");
    }
}


/* Read unsigned char safely */
unsigned char read_unsigned_char()
{
    char input[100];
    char extra;
    unsigned int value;

    while (1)
    {
        read_line(input, sizeof(input));

        if (sscanf(input, "%u %c", &value, &extra) == 1 &&
            value <= 255)
        {
            return (unsigned char)value;
        }

        printf("Invalid input. Enter a number from 0 to 255: ");
    }
}


/* Read menu choice */
int read_choice()
{
    char input[100];
    char extra;
    int value;

    while (1)
    {
        read_line(input, sizeof(input));

        if (sscanf(input, "%d %c", &value, &extra) == 1)
        {
            return value;
        }

        printf("Invalid choice. Enter a number: ");
    }
}


/* =========================================================
                       MEMBER 1
                    ORDER MANAGEMENT
   ========================================================= */

/*
   Member 1 responsibilities:

   1. Place a new order
   2. Store order information
   3. Validate duplicate Order ID
   4. Display all orders
*/


/* Check duplicate Order ID */
int order_id_exists(unsigned int id)
{
    for (unsigned int i = 0; i < orderCount; i++)
    {
        if (orders[i].order_id == id)
        {
            return 1;
        }
    }

    return 0;
}


/* Place a new order */
void place_order()
{
    unsigned int id;

    if (orderCount >= MAX_ORDERS)
    {
        printf("\nOrder limit reached!\n");
        return;
    }

    printf("\n========== PLACE NEW ORDER ==========\n");

    printf("Enter Order ID: ");
    id = read_unsigned_int();

    /* Duplicate ID validation */
    if (order_id_exists(id))
    {
        printf("\nError: Order ID already exists!\n");
        return;
    }

    orders[orderCount].order_id = id;

    printf("Enter Customer Name: ");
    read_line(orders[orderCount].customer,
              sizeof(orders[orderCount].customer));

    if (strlen(orders[orderCount].customer) == 0)
    {
        printf("Customer name cannot be empty!\n");
        return;
    }

    printf("Enter Number of Items (1-10): ");

    while (1)
    {
        orders[orderCount].item_count = read_unsigned_char();

        if (orders[orderCount].item_count >= 1 &&
            orders[orderCount].item_count <= MAX_ITEMS)
        {
            break;
        }

        printf("Please enter a number from 1 to 10: ");
    }


    /* Enter item information */
    for (unsigned int i = 0;
         i < orders[orderCount].item_count;
         i++)
    {
        printf("Enter Item ID %u: ", i + 1);

        orders[orderCount].item_ids[i] =
            read_unsigned_int();

        printf("Enter Quantity: ");

        orders[orderCount].qty[i] =
            read_unsigned_char();

        while (orders[orderCount].qty[i] == 0)
        {
            printf("Quantity must be greater than 0: ");

            orders[orderCount].qty[i] =
                read_unsigned_char();
        }
    }


    /* New order starts as Pending */
    strcpy(orders[orderCount].status, "Pending");

    orderCount++;

    printf("\nOrder added successfully!\n");

    /* Automatically save after adding */
    /* Member 1 can call save_data() */
}


/* Display all orders */
void display_orders()
{
    if (orderCount == 0)
    {
        printf("\nNo orders available.\n");
        return;
    }

    printf("\n========== ALL ORDERS ==========\n");

    for (unsigned int i = 0; i < orderCount; i++)
    {
        printf("\n--------------------------------\n");

        printf("Order ID : %u\n",
               orders[i].order_id);

        printf("Customer : %s\n",
               orders[i].customer);

        printf("Status   : %s\n",
               orders[i].status);

        printf("Items:\n");

        for (unsigned int j = 0;
             j < orders[i].item_count;
             j++)
        {
            printf("  Item ID: %u | Quantity: %u\n",
                   orders[i].item_ids[j],
                   (unsigned int)orders[i].qty[j]);
        }
    }

    printf("\n--------------------------------\n");
}


/* =========================================================
                       FILE PERSISTENCE
   ========================================================= */

/*
   The assignment requires data persistence.

   save_data() writes orders to orders.dat.
   load_data() reads orders from orders.dat.
*/


void save_data()
{
    FILE *file;

    file = fopen(DATA_FILE, "wb");

    if (file == NULL)
    {
        printf("\nError: Could not save data.\n");
        return;
    }

    fwrite(&orderCount,
           sizeof(orderCount),
           1,
           file);

    fwrite(orders,
           sizeof(struct Order),
           orderCount,
           file);

    fclose(file);

    printf("\nData saved successfully.\n");
}


void load_data()
{
    FILE *file;

    file = fopen(DATA_FILE, "rb");

    if (file == NULL)
    {
        printf("\nNo previous data found.\n");
        return;
    }

    fread(&orderCount,
          sizeof(orderCount),
          1,
          file);

    if (orderCount > MAX_ORDERS)
    {
        orderCount = 0;

        fclose(file);

        printf("\nInvalid data file.\n");
        return;
    }

    fread(orders,
          sizeof(struct Order),
          orderCount,
          file);

    fclose(file);

    printf("\n%u orders loaded successfully.\n",
           orderCount);
}


/* =========================================================
                       MEMBER 2
                   STATUS MANAGEMENT
   ========================================================= */

/*
   Member 2 responsibilities:

   Pending -> Packed -> Shipped

   An order cannot move backward.
*/


void update_status()
{
    unsigned int id;
    char new_status[12];

    printf("\n========== UPDATE ORDER STATUS ==========\n");

    printf("Enter Order ID: ");

    id = read_unsigned_int();


    for (unsigned int i = 0;
         i < orderCount;
         i++)
    {
        if (orders[i].order_id == id)
        {
            printf("\nCustomer      : %s\n",
                   orders[i].customer);

            printf("Current Status: %s\n",
                   orders[i].status);


            /* Pending -> Packed */

            if (strcmp(orders[i].status, "Pending") == 0)
            {
                printf("Allowed Status: Packed\n");

                printf("Enter New Status: ");

                read_line(new_status,
                          sizeof(new_status));

                if (strcmp(new_status, "Packed") == 0)
                {
                    strcpy(orders[i].status, "Packed");

                    printf("\nStatus Updated Successfully!\n");

                    save_data();
                }
                else
                {
                    printf("\nInvalid Status Transition!\n");
                }

                return;
            }


            /* Packed -> Shipped */

            else if (strcmp(orders[i].status, "Packed") == 0)
            {
                printf("Allowed Status: Shipped\n");

                printf("Enter New Status: ");

                read_line(new_status,
                          sizeof(new_status));

                if (strcmp(new_status, "Shipped") == 0)
                {
                    strcpy(orders[i].status, "Shipped");

                    printf("\nStatus Updated Successfully!\n");

                    save_data();
                }
                else
                {
                    printf("\nInvalid Status Transition!\n");
                }

                return;
            }


            /* Already shipped */

            else
            {
                printf("\nOrder is already Shipped.\n");
                return;
            }
        }
    }

    printf("\nOrder Not Found!\n");
}


/* =========================================================
                       MEMBER 3
                    SEARCH & REPORTS
   ========================================================= */

/*
   Member 3 responsibilities:

   1. Search by customer OR status
   2. Generate daily shipment report
*/


void search_by_customer_or_status()
{
    char search[50];
    int found = 0;

    printf("\n========== SEARCH ORDERS ==========\n");

    printf("Enter Customer Name or Status: ");

    read_line(search, sizeof(search));


    if (strlen(search) == 0)
    {
        printf("\nSearch cannot be empty.\n");
        return;
    }


    for (unsigned int i = 0;
         i < orderCount;
         i++)
    {
        /*
           strstr() allows partial customer matching.

           Example:
           Customer = John Doe
           Search = John

           Result = Found
        */

        if (strstr(orders[i].customer, search) != NULL ||
            strstr(orders[i].status, search) != NULL)
        {
            printf("\n----------------------------\n");

            printf("Order ID : %u\n",
                   orders[i].order_id);

            printf("Customer : %s\n",
                   orders[i].customer);

            printf("Status   : %s\n",
                   orders[i].status);

            found = 1;
        }
    }


    if (!found)
    {
        printf("\nNo matching order found.\n");
    }
}


/* Daily shipment report */
void daily_shipment_report()
{
    unsigned int shipped = 0;

    printf("\n========== DAILY SHIPMENT REPORT ==========\n");


    for (unsigned int i = 0;
         i < orderCount;
         i++)
    {
        if (strcmp(orders[i].status, "Shipped") == 0)
        {
            shipped++;

            printf("\nOrder ID : %u\n",
                   orders[i].order_id);

            printf("Customer : %s\n",
                   orders[i].customer);
        }
    }


    printf("\nTotal Shipped Orders: %u\n",
           shipped);
}


/* =========================================================
                         MAIN MENU
   ========================================================= */


int main()
{
    int choice;

    /* Load previous orders when program starts */
    load_data();


    while (1)
    {
        printf("\n========================================\n");
        printf("       WAREHOUSE ORDER TRACKER\n");
        printf("========================================\n");

        printf("1. Place New Order\n");
        printf("2. Update Order Status\n");
        printf("3. Search Customer or Status\n");
        printf("4. Daily Shipment Report\n");
        printf("5. Display All Orders\n");
        printf("6. Save Data\n");
        printf("7. Exit\n");

        printf("\nEnter your choice: ");

        choice = read_choice();


        /* ================= MEMBER 1 ================= */

        if (choice == 1)
        {
            place_order();

            /* Save new order */
            save_data();
        }


        /* ================= MEMBER 2 ================= */

        else if (choice == 2)
        {
            update_status();
        }


        /* ================= MEMBER 3 ================= */

        else if (choice == 3)
        {
            search_by_customer_or_status();
        }


        /* ================= MEMBER 3 ================= */

        else if (choice == 4)
        {
            daily_shipment_report();
        }


        /* ================= MEMBER 1 ================= */

        else if (choice == 5)
        {
            display_orders();
        }


        /* ================= FILE PERSISTENCE ================= */

        else if (choice == 6)
        {
            save_data();
        }


        /* ================= MAIN ================= */

        else if (choice == 7)
        {
            /*
               Save before exiting so no data is lost.
            */

            save_data();

            printf("\nProgram ended.\n");

            break;
        }


        else
        {
            printf("\nInvalid choice!\n");
        }
    }


    return 0;
}