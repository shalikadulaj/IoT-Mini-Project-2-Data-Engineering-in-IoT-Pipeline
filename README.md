# :octocat: IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline

---

### Group Members

* Shalika Dulaj Amarathunga
* Kavinda Rathnayaka

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

## Data Management and Visualization

## :cloud: AWS 

[Amazon Web Services (AWS)](https://docs.aws.amazon.com/index.html ) is a leading cloud platform offering a comprehensive suite of services for data storage, visualization, and alerting. 

### ( click the arrow to open guidelines for each step )

<details>



<summary> AWS IoT Core </summary>

AWS IoT Core is a managed cloud service that facilitates secure communication between IoT devices and the AWS Cloud. It ensures encrypted connectivity, device management, and seamless integration with AWS services. With features like device shadows and a scalable architecture, it's ideal for building secure and scalable IoT applications. 

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



Now need to add the End point to the code. You can get the Endpoint from the below path 

	AWS IoT  > Settings > Device data endpoint 

[Go back to step 8](#aftercreate)




At this moment you can check whether the data is receiving. If not, you have to check the above steps again. To check follow the below steps. 

	AWS IoT > Test > MQTT test client > Subscribe to a topic (sensor/station1) > Subscribe 

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
	"timestamp": "2023-12-24 00:22:21",
	"site": "1",
	"node": "3",
	"temperature": "41",
	}

First, you need to add rules. Follow below steps to add rules 

	AWS IoT > Message Routing > Rules > Create rule 

- Specify rule properties 

- Configure SQL statement 
	- Write this quarry to select all the data coming from the topic, and ingest to the timestream. 

			SELECT * FROM 'sensor/station1'   

- Attach rule actions - This is the action when receiving data. 
	- Select - “Timestream table (write message into a Timestream table)” 
	- Add database - If you have not created a database, you can create a database by clicking on “Create Timetream database”. Select standard database. 
	- Add Table – Click on create timestream table 
	- Add a IAM role – Click on create new role 

- Review and create 
</details>

<details>

<summary> AWS Managed Grafana </summary>


AWS Managed Grafana is a fully managed and scalable service that simplifies the deployment, operation, and scaling of Grafana for analytics and monitoring. It integrates seamlessly with other AWS services, offering a user-friendly interface for creating dashboards and visualizations to gain insights from diverse data sources. We are using Grafana for visualizing data using AWS Timestream as a data source. 

You can create the workspace as below 

	Amazon Managed Grafana > All workspaces > Create workspace 

- Specify workspace details 
	- Give a unique name 
	- Select Grafana version – We are using 8.4  

 

- Configure settings 
	- Select Authentication access - “AWS IAM Identity Center (successor to AWS SSO)” 

- Service managed permission settings 
	- Select data sources  - “Amazon TimeStream” 

- Review and create 

**Creating user**

	Amazon Managed Grafana > All workspaces > Select workspace created above > Authentication > Assign new user or group > Select User > Action > Make admin 

If you can't find a user, you have to add a user by below method 

	IAM Identity Center > Users >  Add user (giving email and other information) 

After adding you can see the user under "configure users" in your workspace 
 

Loging to Grafana workspace 

	Amazon Managed Grafana > All workspaces > Select workspace created above >  Click on “Grafana workspace URL” 

Sign in with AWS SSO 

	Add Data Source > Select Amazon Timestream > Select default region (should be equal to Endpoint region) 

Add data base, table and measure. Then save.

Now you are successfully connected the data source. Then using Grafana, you can create a dashboard as you need. 

</details>

<details>

<summary> AWS SNS </summary>


Amazon Simple Notification Service (SNS) is a fully managed messaging service by AWS. It enables the publishing of messages to a variety of endpoints, including mobile devices, email, and more. SNS simplifies the creation and management of message-driven applications, providing flexibility and scalability for diverse communication scenarios. In here rules are set for triggering email alerts using AWS SNS service. To set rules follow below steps. 

	AWS IoT > Message Routing > Rules > Create rule 

- Specify rule properties 

- Configure SQL statement 
	- Write this quarry to select  data once the temperature value is greater than 50. And data will be sent to the topic.  


			SELECT *,timestamp() as ts FROM 'sensor/station1' WHERE temperature > 50 

- Attach rule actions - This is the action when receiving data. 
	- Select - “Simple Notification Service (SNS)” 
	- Add SNS topic – Create SNS topic 
	- Add a IAM role – Click on create new role 

- Review and create 

**Create a subscription**

	Amazon SNS > Topics > Select notification which you create above > Create subscription 

- Select protocol as “Email”. If you need to send any notification, you select any other option. 
	- Add Endpoint – email address  
	- Create Subscription 

- Login to email and confirm the subscription 
 

Here rules are set for triggering email alerts using the AWS SNS service. Write SQL quarries to trigger AWS SNS 

</details>


<details>
<summary> AWS Dynamodb </summary>



AWS DynamoDB, a fully managed NoSQL database, we are using this for storing alert data. With seamless scalability and low-latency access, DynamoDB ensures reliable and fast retrieval of alert information. Its flexible schema accommodates evolving data needs, making it a robust solution for storing and retrieving dynamic alert data. 

**Create rule for ingesting alert data into dynamoDB**

	AWS IoT > Message Routing > Rules > Select rule which you create above (for sending email) > Edit
- Add another rule action  
	- Choose an action - “DyanamoDBv2 (Split message into multiple colums of a DynamoDB table” 
	- Add table – Click on “Create DynamoDB table” 
	- Add a IAM role – Click on “create new role” 

</details>







