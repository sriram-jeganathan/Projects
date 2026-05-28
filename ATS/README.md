SmartATS - Intelligent Applicant Tracking System

AI-powered ATS built with Spring Boot 3, AI integration, and Milvus vector database.

--------------------------------------------------
TECH STACK
--------------------------------------------------

- Java 21
- Spring Boot 3.2.5
- Spring AI
- MySQL 8
- Redis
- RabbitMQ
- MinIO
- Milvus Vector Database
- MyBatis-Plus
- JWT Authentication
- Apache POI
- PDFBox
- Docker Compose
- JUnit 5
- Mockito
- MockMvc

--------------------------------------------------
PROJECT OVERVIEW
--------------------------------------------------

SmartATS is an AI-driven recruitment management system designed for HR and recruitment workflows.

The system supports:
- Resume uploads
- AI-based resume parsing
- Candidate management
- Semantic candidate search
- Job applications
- Interview scheduling
- Webhook notifications
- Distributed asynchronous processing

This project demonstrates:
- Spring Boot backend development
- AI integration
- Distributed system architecture
- Vector database search
- Scalable ATS workflow design

--------------------------------------------------
CORE FEATURES
--------------------------------------------------

1. AI Resume Parsing
- Upload PDF/DOC/DOCX resumes
- Extract:
  - Name
  - Skills
  - Experience
  - Education
- Structured candidate data generation

2. Asynchronous Processing Pipeline
- RabbitMQ message queues
- Distributed locking using Redisson
- Retry mechanism
- Dead-letter queue handling

3. Semantic Candidate Search
- Milvus vector database
- Embedding-based search
- Natural language candidate retrieval

4. Recruitment Workflow
- Job posting
- Candidate management
- Application tracking
- Interview scheduling
- Feedback management

5. Webhook Notifications
- Event-driven notifications
- HMAC-SHA256 signature verification
- Retry support
- Asynchronous delivery

6. Redis Caching System
- Cache-aside strategy
- Delayed double deletion
- Atomic counters
- Ranking support

--------------------------------------------------
SYSTEM WORKFLOW
--------------------------------------------------

Resume Upload
    ->
MD5 Duplicate Check
    ->
MinIO Storage
    ->
RabbitMQ Async Queue
    ->
AI Resume Parsing
    ->
Vector Embedding
    ->
Milvus Storage
    ->
Semantic Search
    ->
Candidate Ranking
    ->
Webhook Notification

--------------------------------------------------
SYSTEM ARCHITECTURE
--------------------------------------------------

Frontend
    ->
Spring Security + JWT Authentication
    ->
REST APIs
    ->
Business Modules

Modules:
- Authentication
- Job Management
- Resume Processing
- Candidate Management
- Applications
- Interviews
- Webhooks
- Smart Search

Infrastructure:
- RabbitMQ
- Redis
- MinIO
- Milvus
- MySQL

--------------------------------------------------
API MODULES
--------------------------------------------------

1. Authentication Module
- Register
- Login
- JWT Refresh
- Email Verification

2. Job Module
- Create jobs
- Update jobs
- Publish jobs
- Close jobs

3. Resume Module
- Resume upload
- Batch upload
- Parsing status tracking

4. Candidate Module
- Candidate CRUD
- Filtering
- Candidate search

5. Application Module
- Create applications
- Status management
- Workflow tracking

6. Interview Module
- Schedule interviews
- Feedback management
- Interview tracking

7. Webhook Module
- Webhook registration
- Event delivery
- Retry support

8. Smart Search Module
- Semantic candidate search
- RAG-based retrieval

--------------------------------------------------
DATABASE TABLES
--------------------------------------------------

- users
- jobs
- resumes
- candidates
- job_applications
- interview_records
- webhook_configs
- webhook_logs

--------------------------------------------------
PROJECT STRUCTURE
--------------------------------------------------

src/main/java/com/smartats/

├── common/
├── config/
├── infrastructure/
├── module/
│   ├── auth/
│   ├── job/
│   ├── resume/
│   ├── candidate/
│   ├── application/
│   ├── interview/
│   └── webhook/

--------------------------------------------------
SECURITY FEATURES
--------------------------------------------------

- Spring Security
- JWT Authentication
- Role-based access control
- Secure file validation
- HMAC webhook signatures

--------------------------------------------------
AI & ML FEATURES
--------------------------------------------------

- Resume parsing
- Candidate skill extraction
- Semantic similarity search
- Embedding generation
- AI-assisted recruitment workflow
- Vector-based candidate ranking

--------------------------------------------------
TESTING
--------------------------------------------------

- 190 test cases
- Unit testing with Mockito
- Integration testing with MockMvc
- Service layer testing
- Controller testing

--------------------------------------------------
DEPLOYMENT
--------------------------------------------------

Docker Compose services:
- MySQL
- Redis
- RabbitMQ
- MinIO
- Milvus

--------------------------------------------------
LEARNING OUTCOMES
--------------------------------------------------

This project demonstrates:
- Enterprise Spring Boot development
- AI-powered recruitment systems
- Distributed architecture
- Semantic search systems
- Event-driven processing
- Vector database integration
- ATS workflow implementation
