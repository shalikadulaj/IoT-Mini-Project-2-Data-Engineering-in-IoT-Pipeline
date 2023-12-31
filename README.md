# :octocat: IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline

---

### Group Members

* *Shalika Dulaj Amarathunga*
* *Kavinda Rathnayaka*

##  Video Demonstration

[Click here for video Demostration]()



## Data Engineering in IoT Pipeline

### Introduction

This project constitutes the second mini-project for the Internet of Things (IoT) course at the University of Oulu. The core objective is developing and implementing an IoT pipeline with a primary focus on data engineering. The central theme involves extracting, pre-processing, visualization, and storing temperature and pressure sensor data obtained from IoT devices deployed across three distinct remote locations. Each location has three temperature sensors, contributing to a comprehensive dataset.

The project initiates the retrieval of temperature data from IoT devices strategically positioned in the three remote locations. These devices serve as sources of real-time temperature measurements, providing valuable insights into environmental conditions at each site. The distributed nature of the deployment, with three sensors from each sensor type at each location, enhances the granularity and comprehensiveness of the collected data.

The data engineering pipeline encompasses crucial stages, beginning with data pre-processing. Raw temperature data is subjected to necessary transformations and cleaning processes to ensure accuracy and consistency. Subsequently, the pre-processed data is channeled into a visualization module, where it is translated into informative visual representations. To facilitate long-term data management, a cloud-based storage system is implemented.


## Tools and Requirement

<div align="center">


#### add flow chart

</div>


 ## Sensing Layer




 ## Network Layer







## Getting Started

### ( click the arrow to open guidelines for each step )

<details>

<summary> STEP 1:Setup FIT IOT-LAB Testbed and Configure The Network environment </summary>

#### logged in to FIT IOT-LAB and ssh to the Grenoble site.

> Note: we recommend that you use Grenoble for this experiment as we are using A8-M3 nodes and there are plenty of boards available.
  
Connect to the SSH frontend of the Grenoble site of FIT/IoT-LAB by using the username you created when you registered with the testbed:

submit an experiment

Step 1:
![Screenshot 2023-12-31 at 11 19 34](https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/assets/153508129/87616d1e-95be-46c3-8290-47290aef46ca)
Step 2:
![Screenshot 2023-12-31 at 11 20 45](https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/assets/153508129/d3af4678-7c8f-4730-b1f4-15e7b6b7936d)
Step 3:
![Screenshot 2023-12-31 at 11 21 49](https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/assets/153508129/320612ba-2805-4a0b-a122-3092c5c5495f)
Step 4: Browse the clone directory on your local PC. ( IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline )
![Screenshot 2023-12-31 at 11 33 51](https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/assets/153508129/54ae61a9-7eb1-476e-8358-44713a2e2f08)

Now Open a Terminal from the front end and follow the below steps.

```ruby
   ssh <username>@grenoble.iot-lab.info
```
Now you can configure the network of the border router on m3-1 and propagate an IPv6 prefix with ethos_uhcpd.py

```ruby
username@grenoble:~$ sudo ethos_uhcpd.py m3-1 tap0 2001:660:5307:3100::1/64
```
The network is finally configured and you will see a similar response below:

```ruby
net.ipv6.conf.tap0.forwarding = 1
net.ipv6.conf.tap0.accept_ra = 0
----> ethos: sending hello.
----> ethos: activating serial pass-through.
----> ethos: hello reply received
```
> Note 1: leave the terminal open (you don’t want to kill ethos_uhcpd.py, it bridges the BR to the front-end network)

> Note 2: If you have an error “Invalid prefix – Network overlapping with routes”, it’s because another experiment is using the same ipv6 prefix
> (e.g. 2001:660:5307:3100::1/64).

</details>




## Data Management and Visualization

## :cloud: AWS 

[Amazon Web Services (AWS)](https://docs.aws.amazon.com/index.html ) is a leading cloud platform offering a comprehensive suite of services for data processing, storage, visualization, and alerting. 

### *( click the arrow to open guidelines for each step )*

<details>



<summary> AWS IoT Core </summary>

AWS IoT Core is a managed cloud service that facilitates secure communication between IoT devices and the AWS Cloud. It ensures encrypted connectivity, device management, and seamless integration with AWS services. With features like device shadows and a scalable architecture, it's ideal for building secure and scalable IoT applications. According to the rule actions it sends data to amazon timestream table and  Lambda function.

https://docs.aws.amazon.com/iot/ 


The border router publishes sensor data from FIT IoT Lab to the specific topic in AWS IoT core. There are rules to control data, which receive to the IoT core.



<details>


<a name="Createathing"> </a>

<summary>  Create a thing and Certificates </summary> 

A thing resource is a digital representation of a physical device or logical entity in AWS IoT. Your device or entity needs a thing resource in the registry to use AWS IoT features such as Device Shadows, events, jobs, and device management features.

Follow the below steps to create a thing 

	AWS IoT Core > Manage > All Device > Things > Create Things 
- Specify thing properties 

- Configure device certificate 

- Attach policies to the certificate 


Finally, you must download the device certificate, key files, and Root CA Certificates. These certificates should be added to the code. It is mentioned in the code, that you can replace the certificates with yours's. 



Now you need to add the Endpoint to the code. You can get the Endpoint from the below path.

	AWS IoT  > Settings > Device data endpoint 




At this moment you can check whether the data is receiving. If not, you have to check the above steps again. To check follow the below steps. 

	AWS IoT > Test > MQTT test client > Subscribe to a topic ("Grenoble/Data") > Subscribe 

Replace the topic with your topic. Now you can see the data is receiving as below. 

<div align="center">


![31 12 2023_08 48 05_REC](https://github.com/shalikadulaj/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/assets/58818511/fb8435d7-c632-4592-b097-862b01c65956)


</div>
</details>

</details>



<details>
<summary> AWS Timestream </summary> 

AWS Timestream is a fully managed, serverless time-series database service provided by Amazon Web Services (AWS). It is specifically designed to handle time-series data at scale. Time-series data is characterized by data points associated with timestamps. In this project, the data from the IoT core is ingested into the AWS Timestream database using AWS rules.

**Ingesting data into Timestream**

Sample JSON data

	{
	    "temperature": 41.0,
  	    "pressure": 983.0,
 	    "site": "Grenoble",
  	    "timestamp": "2023-12-31 01:57:07"
	}

First, you need to add rules. Follow the below steps to add rules 

	AWS IoT > Message Routing > Rules > Create rule 

- Specify rule properties 

- Configure SQL statement 
	- Write this quarry to select all the data coming from the topic, and ingest to the timestream. 

			SELECT * FROM 'Grenoble/Data'   

*Note - In this project, data comes from three sites (Grenoble, Saclay, Paris). We get the processed data from the 'Grenoble/Data', 'Saclay/Data', and 'Paris/Data' topics . And we get unprocessed data from each node (there are 9 nodes). Use sensor/node1, sensor/node2, sensor/node3 ... etc, topics to get noisy data (before preprocessing). We use this noisy data only for visualizing purposes. We do not store this noisy data in the DynamoDB database.*


- Attach rule actions - This is the action when receiving data. 
	- Select - “Timestream table (write message into a Timestream table)” 
	- Add database - If you have not created a database, you can create a database by clicking on “Create Timetream database”. Select standard database. 
	- Add Table – Click on "create timestream table" 
	- Add an "IAM role" – Click on create new role 

- Review and create 
</details>


<details>


<summary> AWS Managed Grafana </summary>


AWS Managed Grafana is a fully managed and scalable service that simplifies the deployment, operation, and scaling of Grafana for analytics and monitoring. It integrates seamlessly with other AWS services, offering a user-friendly interface for creating dashboards and visualizations to gain insights from diverse data sources. We are using Grafana for visualizing data using AWS Timestream as a data source.

You can create the workspace as below 

	Amazon Managed Grafana > All workspaces > Create workspace 

- Specify workspace details 
	- Give a unique name 
	- Select Grafana version – We are using Version 8.4
  

 

- Configure settings 
	- Select Authentication access - “AWS IAM Identity Center (successor to AWS SSO)” 

- Service managed permission settings 
	- Select data sources  - “Amazon TimeStream” 

- Review and create 

**Creating user**

	Amazon Managed Grafana > All workspaces > Select workspace created above > Authentication > Assign new user or group > Select User > Action > Make admin 

If you can't find a user, you have to add a user by the below method 

	IAM Identity Center > Users >  Add user (giving email and other information) 

After adding you can see the user under "configure users" in your workspace 
 

Login to Grafana workspace 

	Amazon Managed Grafana > All workspaces > Select workspace created above >  Click on “Grafana workspace URL” 

Sign in with AWS SSO 

	Add Data Source > Select Amazon Timestream > Select default region (should be equal to Endpoint region) 

 We are using the “US East (N. Virginia) us-east-1” region. Add database, table, and measure. Then save. Now you are successfully connected to the data source. Then using Grafana, you can create a dashboard as you need. 

</details>







<details>
<summary> AWS DynamoDB </summary>



AWS DynamoDB, a fully managed NoSQL database, it is used for storing all the processed data. With seamless scalability and low-latency access, DynamoDB ensures reliable and fast retrieval of alert information. Its flexible schema accommodates evolving data needs, making it a robust solution for storing and retrieving dynamic data.


**To create a DynamoDB database follow the below steps**

	
- Search DynamoDB in the AWS console
  
  		tables> Create table
 
	- Provide table details (table name, partition key) 
	- create a table with default settings. 
	
When you are writing the code for the lambda function, this table name will be required.

</details>



<details>

<summary> AWS Lambda - Cloud Data Preprocessing </summary>

AWS Lambda is a serverless computing service provided by Amazon Web Services (AWS). It allows developers to run code without the need to provision or manage servers. This serverless architecture enables developers to focus solely on writing code to meet business requirements, without worrying about the underlying infrastructure.

In the architecture designed for our data processing workflow, we leverage AWS Lambda to seamlessly transmit data to DynamoDB. Before storing this data in the DynamoDB database, a crucial step is introduced within the Lambda function itself to address potential noise or missing values. While initial data preprocessing is performed in the node, noise can be generated during transmission. To mitigate this, the Lambda function incorporates a dedicated data preprocessing stage just before the data is committed to the database. This ensures that any discrepancies or inconsistencies in the incoming data are systematically rectified. The preprocessing logic, housed within the Lambda function, allows us to tailor the data precisely before persisting it in DynamoDB. This approach not only fortifies the integrity of the stored information but also streamlines the entire data-handling process within the serverless architecture.


**To create a lambda function follow the below steps**

- Search AWS lambda in aws console

		Dashboard > Create function 


	- Select -Author from scratch
	 - Add basic information -  (Function name-“LambdaFunction”)
	 - Runtime - Python 3.12
 	- Architecture x86_64
	 - Click on the Create function

Now you have a function. Then need to link the trigger with the function. There are two options. You can use any option. The first one is, to click on add trigger button and select a source. You may select  AWS IoT as the source. Because this function receives sensor data through AWS IoT.
The second one is,

	AWS IoT > Message Routing > Rules > Create rule 

- Specify rule properties
- Configure SQL statement
	- Write this quarry to select all the data coming from the topic, and ingest to the lambda function.

			SELECT * FROM 'Grenoble/Data'   


*Note - in this project, data comes from three sites (Grenoble, Saclay, Paris).*



- Attach rule actions - This is the action when receiving data.

	- Select - “Lambda (send a message to a Lambda function)”
	- Lambda function - select the function that you created in the above step (“LambdaFunction”) 
	- Click next and create


Now you can start coding on lambda_function.py. The data processing method in this Lambda function focuses on handling outliers in temperature and pressure data before storing it in DynamoDB. Using the Interquartile Range (IQR) method, the function identifies outliers, replacing values beyond calculated thresholds with the nearest threshold value. Based on your requirements you can add any kind of data processing algorithm here. This ensures that extreme data points do not skew the dataset. The function then constructs a DynamoDB database with both original and processed values for temperature and pressure, contributing to the overall robustness of the stored data.



When you run this code you will get a permission error. To solve it follow the below steps.


	IAM > Click on Roles > create role > AWS service >choose service as DynamoDB > Next > Add “AmazonDynamoDBFullAccess” policy > next > give role name > click on create role



Then go back to the lambda,

		Configuration > permission > Edit Execution role > Select the role just created > save

Now all the data received from each topic will be processed and stored in DynamoDB.

   

</details>




