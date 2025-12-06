import boto3
from moto import mock_dynamodb
import pytest


TABLE_NAME = "Test-Chess"


@pytest.fixture
def dynamodb_table():
    with mock_dynamodb():
        # Create mocked DynamoDB
        dynamodb = boto3.resource("dynamodb", region_name="us-east-1")

        # Create table
        table = dynamodb.create_table(
            TableName=TABLE_NAME,
            KeySchema=[
                {"AttributeName": "sub", "KeyType": "HASH"},   # partition key
                {"AttributeName": "game_id", "KeyType": "RANGE"},  # sort key
            ],
            AttributeDefinitions=[
                {"AttributeName": "sub", "AttributeType": "S"},
                {"AttributeName": "game_id", "AttributeType": "S"},
            ],
            BillingMode="PAY_PER_REQUEST",
        )

        table.wait_until_exists()
        yield table


def test_put_and_get_item(dynamodb_table):
    table = dynamodb_table

    # Sample record
    item = {
        "sub": "user123",
        "game_id": "game456",
        "result": "win",
        "score": 1200,
    }

    # Put item
    table.put_item(Item=item)

    # Get item
    response = table.get_item(
        Key={
            "sub": "user123",
            "game_id": "game456"
        }
    )

    # Assertions
    assert "Item" in response
    assert response["Item"]["result"] == "win"
    assert response["Item"]["score"] == 1200
